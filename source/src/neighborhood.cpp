#include "neighborhood.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "utils.h"


std::tuple<orcs::Schedule, double> orcs::Shift::best_improvement(const orcs::Problem& problem,
        const std::tuple<orcs::Schedule, double>& entry) {

    // Keep the best neighbor
    std::tuple<Schedule, double> best_neighbor = entry;

    // Evaluate all neighbors
    for (int l = 0; l <= problem.m; ++l) {
        for (int idx_origin = 0; idx_origin < std::get<Schedule>(entry)[l].size(); ++idx_origin) {

            // Create a copy as base to perform movements
            Schedule base_schedule = std::get<Schedule>(entry);
            Maneuver maneuver = base_schedule[l][idx_origin];
            base_schedule[l].erase(base_schedule[l].begin() + idx_origin);

            // Evaluate neighbor solutions
            for (int idx_target = 0; idx_target <= base_schedule[l].size(); ++idx_target) {
                if (idx_target != idx_origin) {

                    Schedule current_schedule = base_schedule;
                    current_schedule[l].insert(current_schedule[l].begin() + idx_target, maneuver);

                    std::tuple<Schedule, double> neighbor = std::make_tuple(std::move(current_schedule), 0.0);
                    bool is_feasible = orcs::utils::evaluate(problem, neighbor);

                    // Check if the current neighbor is better
                    if (is_feasible) {
                        if (std::get<double>(neighbor) < std::get<double>(best_neighbor)) {
                            best_neighbor = std::move(neighbor);
                        }
                    }
                }
            }

        }
    }

    // Return the best neighbor
    return best_neighbor;
}

std::tuple<orcs::Schedule, double> orcs::Shift::first_improvement(const orcs::Problem& problem,
        const std::tuple<orcs::Schedule, double>& entry, std::mt19937& generator) {

    // Create a list of all possible movements
    std::vector< std::tuple<int, int, int> > movements;
    for (int l = 0; l <= problem.m; ++l) {
        for (int idx_origin = 0; idx_origin < std::get<Schedule>(entry)[l].size(); ++idx_origin) {
            for (int idx_target = 0; idx_target <= std::get<Schedule>(entry)[l].size() - 1; ++idx_target) {
                if (idx_target != idx_origin) {
                    movements.emplace_back(l, idx_origin, idx_target);
                }
            }
        }
    }

    // Shuffle the list of possible movements
    std::shuffle(movements.begin(), movements.end(), generator);

    // Keep the best neighbor
    std::tuple<Schedule, double> best_neighbor = entry;

    // Evaluate neighbors until find a first one that improves the objective function
    for (const auto& movement : movements) {

        // Get the attributes of the movemnt
        int l, idx_origin, idx_target;
        std::tie(l, idx_origin, idx_target) = movement;

        // Evalutate the movement
        Schedule current_schedule = std::get<Schedule>(entry);
        Maneuver maneuver = current_schedule[l][idx_origin];
        current_schedule[l].erase(current_schedule[l].begin() + idx_origin);
        current_schedule[l].insert(current_schedule[l].begin() + idx_target, maneuver);

        std::tuple<Schedule, double> neighbor = std::make_tuple(std::move(current_schedule), 0.0);
        bool is_feasible = orcs::utils::evaluate(problem, neighbor);

        // Check if the current neighbor is better
        if (is_feasible) {
            if (std::get<double>(neighbor) < std::get<double>(best_neighbor)) {
                best_neighbor = std::move(neighbor);
                break;
            }
        }

    }

    // Return the first improvement neighbor
    return best_neighbor;
}

std::tuple<orcs::Schedule, double> orcs::Shift::shake(const orcs::Problem& problem,
        const std::tuple<orcs::Schedule, double>& entry, bool feasible_only,
        std::mt19937& generator) {

    // Create a list of all possible movements
    std::vector< std::tuple<int, int, int> > movements;
    for (int l = 0; l <= problem.m; ++l) {
        for (int idx_origin = 0; idx_origin < std::get<Schedule>(entry)[l].size(); ++idx_origin) {
            for (int idx_target = 0; idx_target <= std::get<Schedule>(entry)[l].size() - 1; ++idx_target) {
                if (idx_target != idx_origin) {
                    movements.emplace_back(l, idx_origin, idx_target);
                }
            }
        }
    }

    // Shuffle the list of possible movements
    std::shuffle(movements.begin(), movements.end(), generator);

    // Keep the neighbor obtained by the shake movement
    std::tuple<Schedule, double> shaked = entry;

    // Evaluate neighbors until find a first one that improves the objective function
    for (const auto& movement : movements) {

        // Get the attributes of the movement
        int l, idx_origin, idx_target;
        std::tie(l, idx_origin, idx_target) = movement;

        // Evaluate the movement
        Schedule current_schedule = std::get<Schedule>(entry);
        Maneuver maneuver = current_schedule[l][idx_origin];
        current_schedule[l].erase(current_schedule[l].begin() + idx_origin);
        current_schedule[l].insert(current_schedule[l].begin() + idx_target, maneuver);

        std::tuple<Schedule, double> neighbor = std::make_tuple(std::move(current_schedule), 0.0);
        bool is_feasible = orcs::utils::evaluate(problem, neighbor);

        // Check if the current neighbor is better
        if (is_feasible || !feasible_only) {
            shaked = std::move(neighbor);
            break;
        }

    }

    // Return the first improvement neighbor
    return shaked;
}


std::tuple<orcs::Schedule, double> orcs::Reassignement::best_improvement(const orcs::Problem& problem,
        const std::tuple<orcs::Schedule, double>& entry) {

    // Keep the best neighbor
    std::tuple<Schedule, double> best_neighbor = entry;

    // Evaluate all neighbors
    for (int l_origin = 1; l_origin <= problem.m; ++l_origin) {
        for (int idx_origin = 0; idx_origin < std::get<Schedule>(entry)[l_origin].size(); ++idx_origin) {

            // Create a base schedule
            Schedule base_schedule = std::get<Schedule>(entry);
            Maneuver operation = base_schedule[l_origin][idx_origin];
            base_schedule[l_origin].erase(base_schedule[l_origin].begin() + idx_origin);

            // Try neighbor solutions
            for (int l_target = 1; l_target <= problem.m; ++l_target) {
                if (l_target != l_origin) {
                    for (int idx_target = 0; idx_target <= base_schedule[l_target].size(); ++idx_target) {

                        Schedule current_schedule = base_schedule;
                        current_schedule[l_target].insert(current_schedule[l_target].begin() + idx_target, operation);

                        std::tuple<Schedule, double> neighbor = std::make_tuple(std::move(current_schedule), 0.0);
                        bool is_feasible = orcs::utils::evaluate(problem, neighbor);

                        // Check if the current neighbor is better
                        if (is_feasible) {
                            if (std::get<double>(neighbor) < std::get<double>(best_neighbor)) {
                                best_neighbor = std::move(neighbor);
                            }
                        }
                    }
                }
            }

        }
    }

    // Return the best neighbor
    return best_neighbor;
}

std::tuple<orcs::Schedule, double> orcs::Reassignement::first_improvement(const orcs::Problem& problem,
        const std::tuple<orcs::Schedule, double>& entry, std::mt19937& generator) {

    // Create a list of all possible movements
    std::vector< std::tuple<int, int, int, int> > movements;
    for (int l_origin = 1; l_origin <= problem.m; ++l_origin) {
        for (int idx_origin = 0; idx_origin < std::get<Schedule>(entry)[l_origin].size(); ++idx_origin) {
            for (int l_target = 1; l_target <= problem.m; ++l_target) {
                if (l_target != l_origin) {
                    for (int idx_target = 0; idx_target <= std::get<Schedule>(entry)[l_target].size(); ++idx_target) {
                        movements.emplace_back(l_origin, idx_origin, l_target, idx_target);
                    }
                }
            }
        }
    }

    // Shuffle the list of possible movements
    std::shuffle(movements.begin(), movements.end(), generator);

    // Keep the best neighbor
    std::tuple<Schedule, double> best_neighbor = entry;

    // Evaluate neighbors until find a first one that improves the objective function
    for (const auto& movement : movements) {

        // Get the attributes of the movemnt
        int l_origin, idx_origin, l_target, idx_target;
        std::tie(l_origin, idx_origin, l_target, idx_target) = movement;

        // Evalutate the movement
        Schedule current_schedule = std::get<Schedule>(entry);
        Maneuver maneuver = current_schedule[l_origin][idx_origin];
        current_schedule[l_origin].erase(current_schedule[l_origin].begin() + idx_origin);
        current_schedule[l_target].insert(current_schedule[l_target].begin() + idx_target, maneuver);

        std::tuple<Schedule, double> neighbor = std::make_tuple(std::move(current_schedule), 0.0);
        bool is_feasible = orcs::utils::evaluate(problem, neighbor);

        // Check if the current neighbor is better
        if (is_feasible) {
            if (std::get<double>(neighbor) < std::get<double>(best_neighbor)) {
                best_neighbor = std::move(neighbor);
                break;
            }
        }
    }

    // Return the first improvement neighbor
    return best_neighbor;
}

std::tuple<orcs::Schedule, double> orcs::Reassignement::shake(const orcs::Problem& problem,
        const std::tuple<orcs::Schedule, double>& entry, bool feasible_only,
        std::mt19937& generator) {

    // Create a list of all possible movements
    std::vector< std::tuple<int, int, int, int> > movements;
    for (int l_origin = 1; l_origin <= problem.m; ++l_origin) {
        for (int idx_origin = 0; idx_origin < std::get<Schedule>(entry)[l_origin].size(); ++idx_origin) {
            for (int l_target = 1; l_target <= problem.m; ++l_target) {
                if (l_target != l_origin) {
                    for (int idx_target = 0; idx_target <= std::get<Schedule>(entry)[l_target].size(); ++idx_target) {
                        movements.emplace_back(l_origin, idx_origin, l_target, idx_target);
                    }
                }
            }
        }
    }

    // Shuffle the list of possible movements
    std::shuffle(movements.begin(), movements.end(), generator);

    // Keep the neighbor obtained by the shake movement
    std::tuple<Schedule, double> shaked = entry;

    // Evaluate neighbors until find a first one that improves the objective function
    for (const auto& movement : movements) {

        // Get the attributes of the movemnt
        int l_origin, idx_origin, l_target, idx_target;
        std::tie(l_origin, idx_origin, l_target, idx_target) = movement;

        // Evalutate the movement
        Schedule current_schedule = std::get<Schedule>(entry);
        Maneuver maneuver = current_schedule[l_origin][idx_origin];
        current_schedule[l_origin].erase(current_schedule[l_origin].begin() + idx_origin);
        current_schedule[l_target].insert(current_schedule[l_target].begin() + idx_target, maneuver);

        std::tuple<Schedule, double> neighbor = std::make_tuple(std::move(current_schedule), 0.0);
        bool is_feasible = orcs::utils::evaluate(problem, neighbor);

        // Check if the current neighbor is better
        if (is_feasible || !feasible_only) {
            shaked = std::move(neighbor);
            break;
        }

    }

    // Return the first improvement neighbor
    return shaked;
}


std::tuple<orcs::Schedule, double> orcs::Swap::best_improvement(const orcs::Problem& problem,
        const std::tuple<orcs::Schedule, double>& entry) {

    // Keep the best neighbor
    std::tuple<Schedule, double> best_neighbor = entry;

    // Evaluate all neighbors
    for (int l1 = 1; l1 <= problem.m; ++l1) {
        if (std::get<Schedule>(entry)[l1].size() > 0) {
            for (int l2 = l1 + 1; l2 <= problem.m; ++l2) {
                if (std::get<Schedule>(entry)[l2].size() > 0) {
                    for (int idx1 = 0; idx1 < std::get<Schedule>(entry)[l1].size(); ++idx1) {
                        for (int idx2 = 0; idx2 < std::get<Schedule>(entry)[l2].size(); ++idx2) {
                            for (int target1 = 0; target1 <= std::get<Schedule>(entry)[l2].size() - 1; ++target1) {
                                for (int target2 = 0; target2 <= std::get<Schedule>(entry)[l1].size() - 1; ++target2) {

                                    Schedule current_schedule = std::get<Schedule>(entry);

                                    Maneuver maneuver1 = current_schedule[l1][idx1];
                                    current_schedule[l1].erase(current_schedule[l1].begin() + idx1);

                                    Maneuver maneuver2 = current_schedule[l2][idx2];
                                    current_schedule[l2].erase(current_schedule[l2].begin() + idx2);

                                    current_schedule[l2].insert(current_schedule[l2].begin() + target1, maneuver1);
                                    current_schedule[l1].insert(current_schedule[l1].begin() + target2, maneuver2);

                                    std::tuple<Schedule, double> neighbor = std::make_tuple(
                                            std::move(current_schedule), 0.0);
                                    bool is_feasible = orcs::utils::evaluate(problem, neighbor);

                                    // Check if the current neighbor is better
                                    if (is_feasible) {
                                        if (std::get<double>(neighbor) < std::get<double>(best_neighbor)) {
                                            best_neighbor = std::move(neighbor);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Return the best neighbor
    return best_neighbor;
}

std::tuple<orcs::Schedule, double> orcs::Swap::first_improvement(const orcs::Problem& problem,
        const std::tuple<orcs::Schedule, double>& entry, std::mt19937& generator) {

    // Create a list of all possible movements
    std::vector< std::tuple<int, int, int, int, int, int> > movements;
    for (int l1 = 1; l1 <= problem.m; ++l1) {
        if (std::get<Schedule>(entry)[l1].size() > 0) {
            for (int l2 = l1 + 1; l2 <= problem.m; ++l2) {
                if (std::get<Schedule>(entry)[l2].size() > 0) {
                    for (int idx1 = 0; idx1 < std::get<Schedule>(entry)[l1].size(); ++idx1) {
                        for (int idx2 = 0; idx2 < std::get<Schedule>(entry)[l2].size(); ++idx2) {
                            for (int target1 = 0; target1 <= std::get<Schedule>(entry)[l2].size() - 1; ++target1) {
                                for (int target2 = 0; target2 <= std::get<Schedule>(entry)[l1].size() - 1; ++target2) {
                                    movements.emplace_back(l1, l2, idx1, idx2, target1, target2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Shuffle the list of possible movements
    std::shuffle(movements.begin(), movements.end(), generator);

    // Keep the best neighbor
    std::tuple<Schedule, double> best_neighbor = entry;

    // Evaluate neighbors until find a first one that improves the objective function
    for (const auto& movement : movements) {

        // Get the attributes of the movemnt
        int l1, l2, idx1, idx2, target1, target2;
        std::tie(l1, l2, idx1, idx2, target1, target2) = movement;

        // Evalutate the movement
        Schedule current_schedule = std::get<Schedule>(entry);

        Maneuver maneuver1 = current_schedule[l1][idx1];
        current_schedule[l1].erase(current_schedule[l1].begin() + idx1);

        Maneuver maneuver2 = current_schedule[l2][idx2];
        current_schedule[l2].erase(current_schedule[l2].begin() + idx2);

        current_schedule[l2].insert(current_schedule[l2].begin() + target1, maneuver1);
        current_schedule[l1].insert(current_schedule[l1].begin() + target2, maneuver2);

        std::tuple<Schedule, double> neighbor = std::make_tuple(std::move(current_schedule), 0.0);
        bool is_feasible = orcs::utils::evaluate(problem, neighbor);

        // Check if the current neighbor is better
        if (is_feasible) {
            if (std::get<double>(neighbor) < std::get<double>(best_neighbor)) {
                best_neighbor = std::move(neighbor);
                break;
            }
        }
    }

    // Return the first improvement neighbor
    return best_neighbor;
}

std::tuple<orcs::Schedule, double> orcs::Swap::shake(const orcs::Problem& problem,
        const std::tuple<orcs::Schedule, double>& entry, bool feasible_only,
        std::mt19937& generator) {

    // Create a list of all possible movements
    std::vector< std::tuple<int, int, int, int, int, int> > movements;
    for (int l1 = 1; l1 <= problem.m; ++l1) {
        if (std::get<Schedule>(entry)[l1].size() > 0) {
            for (int l2 = l1 + 1; l2 <= problem.m; ++l2) {
                if (std::get<Schedule>(entry)[l2].size() > 0) {
                    for (int idx1 = 0; idx1 < std::get<Schedule>(entry)[l1].size(); ++idx1) {
                        for (int idx2 = 0; idx2 < std::get<Schedule>(entry)[l2].size(); ++idx2) {
                            for (int target1 = 0; target1 <= std::get<Schedule>(entry)[l2].size() - 1; ++target1) {
                                for (int target2 = 0; target2 <= std::get<Schedule>(entry)[l1].size() - 1; ++target2) {
                                    movements.emplace_back(l1, l2, idx1, idx2, target1, target2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Shuffle the list of possible movements
    std::shuffle(movements.begin(), movements.end(), generator);

    // Keep the neighbor obtained by the shake movement
    std::tuple<Schedule, double> shaked = entry;

    // Evaluate neighbors until find a first one that improves the objective function
    for (const auto& movement : movements) {

        // Get the attributes of the movemnt
        int l1, l2, idx1, idx2, target1, target2;
        std::tie(l1, l2, idx1, idx2, target1, target2) = movement;

        // Evalutate the movement
        Schedule current_schedule = std::get<Schedule>(entry);

        Maneuver maneuver1 = current_schedule[l1][idx1];
        current_schedule[l1].erase(current_schedule[l1].begin() + idx1);

        Maneuver maneuver2 = current_schedule[l2][idx2];
        current_schedule[l2].erase(current_schedule[l2].begin() + idx2);

        current_schedule[l2].insert(current_schedule[l2].begin() + target1, maneuver1);
        current_schedule[l1].insert(current_schedule[l1].begin() + target2, maneuver2);

        std::tuple<Schedule, double> neighbor = std::make_tuple(std::move(current_schedule), 0.0);
        bool is_feasible = orcs::utils::evaluate(problem, neighbor);

        // Check if the current neighbor is better
        if (is_feasible || !feasible_only) {
            shaked = std::move(neighbor);
            break;
        }

    }

    // Return the first improvement neighbor
    return shaked;
}
