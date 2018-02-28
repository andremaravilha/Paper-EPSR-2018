#include "heur_ils.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <list>

#include <cxxtimer.hpp>

#include "heur_greedy.h"
#include "local_search.h"
#include "neighborhood.h"
#include "utils.h"


std::tuple<orcs::Schedule, double> orcs::ILS::solve(const Problem& problem,
        const cxxproperties::Properties* opt_input, cxxproperties::Properties* opt_output) {

    // Algorithm parameters
    cxxproperties::Properties opt_aux;
    if (opt_input == nullptr) {
        opt_input = &opt_aux;
    }

    const bool verbose = opt_input->get<bool>("verbose", false);
    const unsigned seed = opt_input->get<unsigned>("seed", 0);
    const double time_limit = opt_input->get<double>("time-limit", std::numeric_limits<double>::max());
    const long iterations_limit = opt_input->get<long>("iterations-limit", std::numeric_limits<long>::max());
    const long perturbation_passes_limit = opt_input->get<long>("perturbation-passes-limit", 15);

    // Initialize the random number generator
    std::mt19937 generator;
    generator.seed(seed);

    // Initialize a timer
    cxxtimer::Timer timer;
    timer.start();

    // Define the list of neigborhoods used by the VND
    std::list<Neighborhood*> neighborhoods = {
            new Shift(),
            new Reassignement(),
            new Swap()
    };

    // Log: header
    log_header(verbose);

    // Build an initial solution
    Greedy greedy;
    std::tuple<Schedule, double> start = greedy.solve(problem);

    // Log the initial solution (before LS)
    log_start(std::get<double>(start),
            timer.count<std::chrono::milliseconds>() / 1000.0,
            verbose);

    // Find a local optimum from the start solution
    std::tuple<Schedule, double> incumbent = LocalSearch::best_improvement_vnd(problem,
            start, neighborhoods);

    // Log the initial solution (after LS)
    log_iteration(0L, std::get<double>(start),
            std::get<double>(start), std::get<double>(incumbent),
            timer.count<std::chrono::milliseconds>() / 1000.0,
            verbose);

    // Start the iterative process
    long iteration = 0;
    long perturbation_passes = 1;
    long iteration_last_improvement = 0;

    while (iteration < iterations_limit &&
           timer.count<std::chrono::seconds>() < time_limit &&
           perturbation_passes <= perturbation_passes_limit) {

        // Increment the iteration counter
        ++iteration;

        // Perturbation phase
        std::tuple<Schedule, double> perturbed = perturb(problem, incumbent, generator);
        for (long i = 1; i < perturbation_passes; ++i) {
            perturbed = perturb(problem, perturbed, generator);
        }

        // VND as local search method
        std::tuple<Schedule, double> trial = LocalSearch::best_improvement_vnd(problem,
                perturbed, neighborhoods);

        // Log: status at current iterations
        log_iteration(iteration, std::get<double>(incumbent),
                std::get<double>(perturbed), std::get<double>(trial),
                timer.count<std::chrono::milliseconds>() / 1000.0,
                verbose);

        // Check for improvements
        if (utils::is_lower(std::get<double>(trial), std::get<double>(incumbent))) {

            // Update the incumbent solution
            incumbent = std::move(trial);
            iteration_last_improvement = iteration;

            // Reset the perturbation level
            perturbation_passes = 1;

        } else {

            // Increase the perturbation level
            ++perturbation_passes;
        }

    }

    // Stop timer
    timer.stop();

    // Log: footer
    log_footer(verbose);

    // // Store optional output
    if (opt_output != nullptr) {
        opt_output->add("Iterations", iteration);
        opt_output->add("Runtime (s)", timer.count<std::chrono::milliseconds>() / 1000.0);
        opt_output->add("Start solution", std::get<double>(start));
        opt_output->add("Iteration of last improvement", iteration_last_improvement);
    }

    // Deallocate resources
    for (auto ptr : neighborhoods) {
        delete ptr;
    }

    // Return the solution built
    return incumbent;
}

std::tuple<orcs::Schedule, double> orcs::ILS::perturb(const Problem& problem, const std::tuple<Schedule, double>& entry, std::mt19937& generator) {

    // Create a copy of the original entry
    std::tuple<Schedule, double> perturbed = entry;
    Schedule& schedule = std::get<Schedule>(perturbed);
    double& makespan = std::get<double>(perturbed);

    // Data structures used by the perturbation method
    std::vector<int> indexes;
    std::vector<int> chain;

    // Initialize and shuffle the chain (order of teams to perform the ejection chain)
    for (int l = 1; l <= problem.m; ++l) {
        chain.push_back(l);
    }

    std::shuffle(chain.begin(), chain.end(), generator);

    // Perform a chain of reassignment moves
    for (int idx = 0; idx < chain.size(); ++idx) {

        // Get the pair of teams to perform the reassignemnt move
        int l_origin = chain[idx];
        int l_target = chain[(idx + 1) % chain.size()];

        if (!schedule[l_origin].empty()) {

            // Choose a switch operation to reassign
            int idx_origin = generator() % schedule[l_origin].size();
            Maneuver operation = schedule[l_origin][idx_origin];
            schedule[l_origin].erase(schedule[l_origin].begin() + idx_origin);

            // Fill the possible indexes
            indexes.clear();
            for (int idx = 0; idx <= schedule[l_target].size(); ++idx) {
                indexes.push_back(idx);
            }

            std::shuffle(indexes.begin(), indexes.end(), generator);

            // Try to perform the movement
            bool success = false;
            for (auto idx_target : indexes) {

                // Perform the movement
                schedule[l_target].insert(schedule[l_target].begin() + idx_target, operation);

                // Check the feasibility of the movement
                bool is_feasible = orcs::utils::evaluate(problem, perturbed);

                if (is_feasible) {
                    success = true;
                    break;

                } else {

                    // Undo the movement
                    schedule[l_target].erase(schedule[l_target].begin() + idx_target);
                }
            }

            // Undo the movement, if not feasible
            if (!success) {
                schedule[l_origin].insert(schedule[l_origin].begin() + idx_origin, operation);
                orcs::utils::evaluate(problem, perturbed);
            }

        }
    }

    return perturbed;
}

void orcs::ILS::log_header(bool verbose) {
    if (verbose) {
        std::printf("---------------------------------------------------------------------\n");
        std::printf("| Iter. |   Before LS  |   After LS   |   Incumbent  |   Time (s)   |\n");
        std::printf("---------------------------------------------------------------------\n");
    }
}

void orcs::ILS::log_footer(bool verbose) {
    if (verbose) {
        std::printf("---------------------------------------------------------------------\n");
    }
}

void orcs::ILS::log_iteration(long iteration, double incumbent, double before_ls, double after_ls, double time, bool verbose) {
    if (verbose) {
        bool new_incumbent = utils::is_lower(after_ls, incumbent);
        std::string status = (new_incumbent ? "*" : " ");
        std::printf("| %s%4ld | %12.3lf | %12.3lf | %12.3lf | %12.3lf |\n",
                    status.c_str(), iteration, before_ls, after_ls,
                    (new_incumbent ? after_ls : incumbent), time);
    }
}

void orcs::ILS::log_start(double start, double time, bool verbose) {
    if (verbose) {
        std::printf("| Start | %12s | %12s | %12.3lf | %12.3lf |\n",
                    "---", "---", start, time);
    }
}
