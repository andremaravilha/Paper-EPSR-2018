#include "utils.h"

#include <cmath>


int orcs::utils::compare(double first, double second) {
    return std::abs(first - second) < THRESHOLD ? 0 : (first < second ? -1 : 1);
}

bool orcs::utils::is_equal(double first, double second) {
    return compare(first, second) == 0;
}

bool orcs::utils::is_greater(double first, double second) {
    return compare(first, second) == 1;
}

bool orcs::utils::is_lower(double first, double second) {
    return compare(first, second) == -1;
}

bool orcs::utils::is_greater_equal(double first, double second) {
    return compare(first, second) != -1;
}

bool orcs::utils::is_lower_equal(double first, double second) {
    return compare(first, second) != 1;
}

bool orcs::utils::evaluate(const orcs::Problem& problem, std::tuple<orcs::Schedule, double>& entry) {

    // Get the schedule and makespan
    Schedule& schedule = std::get<Schedule>(entry);
    double& makespan = std::get<double>(entry);

    // Reset the makespan
    makespan = 0.0;

    // Data structures used for checking feasibility and fixing the operation moments
    std::vector<int> index(problem.m + 1, 0);       // next index to analyse of each schedule
    std::vector<int> location(problem.m + 1, 0);    // current location of each team
    std::vector<int> pendings(problem.n + 1, 0);    // number of pending predecessors switch operations
    std::vector<double*> t(problem.n + 1, nullptr); // new operation moment (fixed)

    double t0 = 0.0;
    t[0] = &t0;

    for (int l = 0; l <= problem.m; ++l) {
        for (int idx = 0; idx < schedule[l].size(); ++idx) {
            pendings[std::get<ATTR_SWITCH>(schedule[l][idx])] = problem.predecessors[std::get<ATTR_SWITCH>(schedule[l][idx])].size();
            t[std::get<ATTR_SWITCH>(schedule[l][idx])] = &std::get<ATTR_MOMENT>(schedule[l][idx]);
            *t[std::get<ATTR_SWITCH>(schedule[l][idx])] = std::numeric_limits<double>::max();
        }
    }

    // Presuppose the solution is feasible
    bool feasibility = true;

    // Analyse the solution
    int n_done = 0;
    while (n_done < problem.n && feasibility) {

        feasibility = false;
        for (int l = 0; l <= problem.m; ++l) {
            if (index[l] < schedule[l].size()) {

                // Get the switch
                int j = std::get<ATTR_SWITCH>(schedule[l][index[l]]);

                if (pendings[j] == 0) {

                    // Get the current location of the team
                    int i = location[l];

                    // Compute the operation moment
                    if (l != 0) {
                        *t[j] = *t[i] + problem.p[i] + problem.c[i][j][l];
                    } else {
                        *t[j] = 0.0;
                    }

                    // Check the precedence constraints
                    for (auto k : problem.predecessors[j]) {
                        *t[j] = std::max(*t[j], *t[k] + problem.p[k]);
                    }

                    // Update the pending counters
                    for (auto k : problem.successors[j]) {
                        --pendings[k];
                    }

                    // Update the makespan
                    makespan = std::max(makespan, *t[j] + problem.p[j]);

                    // Update the index and team's location
                    ++index[l];
                    location[l] = j;

                    // Increment the number of switch operations analysed
                    ++n_done;

                    // The solution is feasible so far
                    feasibility = true;
                }
            }
        }
    }

    // If not feasible, makespan is set to infinity
    if (!feasibility) {
        makespan = std::numeric_limits<double>::max();
    }

    return feasibility;
}
