#include "local_search.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "utils.h"


std::tuple<orcs::Schedule, double> orcs::LocalSearch::best_improvement(const Problem& problem,
        const std::tuple<Schedule, double>& entry, Neighborhood& neighborhood) {

    // Keep the best solution found
    std::tuple<Schedule, double> incumbent = entry;

    // Perform the local search
    bool stop = false;
    while (!stop) {

        // Get a neighbor
        std::tuple<Schedule, double> neighbor = neighborhood.best_improvement(problem, incumbent);

        // Check for improvements
        if (utils::is_lower(std::get<double>(neighbor), std::get<double>(incumbent))) {
            incumbent = std::move(neighbor);
        } else {
            stop = true;
        }
    }

    // Return the best solution found
    return incumbent;
}

std::tuple<orcs::Schedule, double> orcs::LocalSearch::first_improvement(const Problem& problem,
        const std::tuple<Schedule, double>& entry, Neighborhood& neighborhood,
        std::mt19937& generator) {

    // Keep the best solution found
    std::tuple<Schedule, double> incumbent = entry;

    // Perform the local search
    bool stop = false;
    while (!stop) {

        // Get a neighbor
        std::tuple<Schedule, double> neighbor = neighborhood.first_improvement(problem,
                incumbent, generator);

        // Check for improvements
        if (utils::is_lower(std::get<double>(neighbor), std::get<double>(incumbent))) {
            incumbent = std::move(neighbor);
        } else {
            stop = true;
        }
    }

    // Return the best solution found
    return incumbent;
}

std::tuple<orcs::Schedule, double> orcs::LocalSearch::best_improvement_vnd(const Problem& problem,
        const std::tuple<Schedule, double>& entry, std::list<Neighborhood*>& neighborhoods) {

    // Keep the best solution found
    std::tuple<Schedule, double> incumbent = entry;

    // Perform the local search
    auto k = neighborhoods.begin();
    while (k != neighborhoods.end()) {

        // Get the neighborhood
        Neighborhood* neighborhood = *k;

        // Get a neighbor
        std::tuple<Schedule, double> neighbor = neighborhood->best_improvement(problem, incumbent);

        // Check for improvements
        if (utils::is_lower(std::get<double>(neighbor), std::get<double>(incumbent))) {

            // Update the incumbent solution
            incumbent = std::move(neighbor);

            // Go to the first neighborhood
            k = neighborhoods.begin();

        } else {

            // Go to the next neighborhood
            ++k;
        }
    }

    // Return the best solution found
    return incumbent;

}

std::tuple<orcs::Schedule, double> orcs::LocalSearch::first_improvement_vnd(const Problem& problem,
        const std::tuple<Schedule, double>& entry, std::list<Neighborhood*>& neighborhoods,
        std::mt19937& generator) {

    // Keep the best solution found
    std::tuple<Schedule, double> incumbent = entry;

    // Perform the local search
    auto k = neighborhoods.begin();
    while (k != neighborhoods.end()) {

        // Get the neighborhood
        Neighborhood* neighborhood = *k;

        // Get a neighbor
        std::tuple<Schedule, double> neighbor = neighborhood->first_improvement(problem,
                incumbent, generator);

        // Check for improvements
        if (utils::is_lower(std::get<double>(neighbor), std::get<double>(incumbent))) {

            // Update the incumbent solution
            incumbent = std::move(neighbor);

            // Go to the first neighborhood
            k = neighborhoods.begin();

        } else {

            // Go to the next neighborhood
            ++k;
        }
    }

    // Return the best solution found
    return incumbent;

}
