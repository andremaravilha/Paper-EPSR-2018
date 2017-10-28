#include "problem.h"
#include <cmath>
#include <fstream>


orcs::Problem::Problem(const std::string& filename) {

    // Open the file
    std::ifstream file(filename.c_str());
    std::string token;

    // Read the problem size
    file >> n;
    file >> m;
    file >> s;

    // Initialize the data structures
    technology = std::vector<Technology>(n + 1, Technology::UNKNOWN);
    action = std::vector<Action>(n + 1, Action::UNKNOWN);
    stage = std::vector<int>(n + 1, 0);

    precedence_matrix = std::vector< std::vector<bool> >(n + 1, std::vector<bool>(n + 1, false));
    predecessors = std::vector< std::set<int> >(n + 1, std::set<int>());
    successors = std::vector< std::set<int> >(n + 1, std::set<int>());

    p = std::vector<double>(n + 1, 0.0);
    c = std::vector< std::vector< std::vector<double> > >(
            n + 1, std::vector< std::vector<double> >(n + 1, std::vector<double>(m + 1, 0.0)));

    // Read switches data
    for (std::size_t i = 1; i <= n; ++i) {

        // Switch ID (just ignore it)
        file >> token;

        // Maneuver time
        file >> token;
        p[i] = std::stod(token);

        // Technology
        file >> token;
        technology[i] = Technology::UNKNOWN;
        if (token.compare("R") == 0) {
            technology[i] = Technology::REMOTE;
        } else if (token.compare("M") == 0) {
            technology[i] = Technology::MANUAL;
        }

        // Action
        file >> token;
        action[i] = Action::UNKNOWN;
        if (token.compare("C") == 0) {
            action[i] = Action::CLOSE;
        } else if (token.compare("O") == 0) {
            action[i] = Action::OPEN;
        }

        // Stage
        file >> token;
        stage[i] = std::stoi(token);

    }

    // Read precedence constraints
    for (std::size_t j = 1; j <= n; ++j) {

        // Switch ID (just ignore it)
        file >> token;

        // Number of predecessors
        file >> token;
        int nprec = std::stoi(token);

        // Predecessors
        for (std::size_t count = 0; count < nprec; ++count) {

            file >> token;
            int i = std::stoi(token);

            predecessors[j].insert(i);
            successors[i].insert(j);
        }
    }

    // Read the travel time matrices
    for (int l = 1; l <= m; ++l) {
        for (int i = 0; i <= n; ++i) {
            for (int j = 0; j <= n; ++j) {
                file >> token;
                c[i][j][l] = std::stod(token);
            }
        }
    }

    // Compute the full precedence matrix
    std::vector<bool> processed(n + 1, false);
    std::set<int> pending;
    for (std::size_t j = 1; j <= n; ++j) {

        // Clear the data structures used
        std::fill(processed.begin(), processed.end(), false);
        pending.clear();

        // Initialize the data structures
        for (auto i : predecessors[j]) {
            pending.insert(i);
        }

        // Compute the precedence graph of j
        while (!pending.empty()) {

            // Get an element from the pending set
            auto iter = pending.begin();
            int i = *iter;
            pending.erase(iter);

            // Update the precedence matrix
            precedence_matrix[i][j] = true;

            // Check i as processed
            processed[i] = true;

            // Add the predecessors of i into the pending set
            for (auto k : predecessors[i]) {
                if (!processed[k]) {
                    pending.insert(k);
                }
            }
        }
    }

    // Close the file
    file.close();
}

double orcs::Problem::evaluate(const Schedule &schedule) const {
    double makespan = 0.0;
    for (int l = 0; l <= m; ++l) {
        for (const Maneuver& maneuver : schedule[l]) {
            makespan = std::max(makespan, std::get<ATTR_MOMENT>(maneuver) + p[std::get<ATTR_SWITCH>(maneuver)]);
        }
    }

    return makespan;
}

bool orcs::Problem::is_feasible(const Schedule &schedule, std::string *msg) const {

    // Check the number of teams
    if (schedule.size() != m + 1) {
        if (msg != nullptr) {
            *msg = "The number of maintenance teams is wrong.";
        }
        return false;
    }

    // Check the switches IDs and if all switches has been assigned to exactly
    // one team.
    std::vector<int> assignment(n + 1, 0);
    for (int l = 0; l <= m; ++l) {
        for (const auto& maneuver : schedule[l]) {
            if (std::get<ATTR_SWITCH>(maneuver) < 1 || std::get<ATTR_SWITCH>(maneuver) > n) {
                if (msg != nullptr) {
                    *msg = "Using invalid switch IDs.";
                }
                return false;
            } else {
                ++assignment[std::get<ATTR_SWITCH>(maneuver)];
            }
        }
    }

    for (int i = 1; i <= n; ++i) {
        if (assignment[i] != 1) {
            if (msg != nullptr) {
                *msg = "There are switches assigned to more than one team or not assigned to any team.";
            }
            return false;
        }
    }

    // Check if all remotely maneuverable switches has been assigned to the
    // dummy team 0
    for (const auto& maneuver : schedule[0]) {
        if (technology[std::get<ATTR_SWITCH>(maneuver)] != Technology::REMOTE) {
            if (msg != nullptr) {
                *msg = "Non-remote controlled switch assigned to dummy team 0.";
            }
            return false;
        }
    }

    // Check if all manually maneuverable switches has been assigned to a team
    for (int l = 1; l <= m; ++l) {
        for (const auto& maneuver : schedule[l]) {
            if (technology[std::get<ATTR_SWITCH>(maneuver)] != Technology::MANUAL) {
                if (msg != nullptr) {
                    *msg = "Non-manual controlled switch assigned to a maintenace team.";
                }
                return false;
            }
        }
    }

    // Check the precedence rules
    std::vector<double> t(n + 1, 0);
    for (int l = 0; l <= m; ++l) {
        for (const auto& maneuver : schedule[l]) {
            t[std::get<ATTR_SWITCH>(maneuver)] = std::get<ATTR_MOMENT>(maneuver);
        }
    }

    for (int j = 1; j <= n; ++j) {
        for (auto i : predecessors[j]) {
            if (t[j] + 1e-5 < t[i]) {
                if (msg != nullptr) {
                    *msg = "Precedence rules violated.";
                }
                return false;
            }
        }
    }

    // Check the consistency of the moments regarding the sequence of maneuvers
    for (int l = 0; l <= m; ++l) {
        if (schedule[l].size() > 1) {
            for (int i = 1; i < schedule[l].size(); ++i) {
                if (std::get<ATTR_MOMENT>(schedule[l][i]) + 1e-5 < std::get<ATTR_MOMENT>(schedule[l][i-1])) {
                    if (msg != nullptr) {
                        *msg = "Moments not consistent to the sequence.";
                    }
                    return false;
                }
            }
        }
    }

    // Check the consistency of the moments regarding the travel times
    for (int l = 1; l <= m; ++l) {
        if (!schedule[l].empty()) {

            // Origin to switch
            if (std::get<ATTR_MOMENT>(schedule[l][0]) + 1e-5 < c[0][std::get<ATTR_SWITCH>(schedule[l][0])][l]) {
                if (msg != nullptr) {
                    *msg = "Moments not consistent to travel times.";
                }
                return false;
            }

            // Switch to switch
            if (schedule[l].size() > 1) {
                for (int i = 1; i < schedule[l].size(); ++i) {
                    if (std::get<ATTR_MOMENT>(schedule[l][i]) + 1e-5 <
                            std::get<ATTR_MOMENT>(schedule[l][i-1]) + p[std::get<ATTR_SWITCH>(schedule[l][i])] +
                            c[std::get<ATTR_SWITCH>(schedule[l][i-1])][std::get<ATTR_SWITCH>(schedule[l][i])][l]) {

                        if (msg != nullptr) {
                            *msg = "Moments not consistent to travel times.";
                        }
                        return false;
                    }
                }
            }
        }
    }

    // All constraints are met
    *msg = "Feasible solution.";
    return true;
}

std::ostream& operator<<(std::ostream& os, orcs::Schedule schedule) {
    os << "REMOTE : < ";
    for (const auto& maneuver : schedule[0]) {
        os << "(" << std::get<orcs::ATTR_SWITCH>(maneuver) << ", "
           << std::get<orcs::ATTR_MOMENT>(maneuver) << ") ";
    }
    os << ">" << std::endl;

    for (int l = 1; l < schedule.size(); ++l) {
        os << "TEAM " << l << " : < ";
        for (const auto& maneuver : schedule[l]) {
            os << "(" << std::get<orcs::ATTR_SWITCH>(maneuver) << ", "
               << std::get<orcs::ATTR_MOMENT>(maneuver) << ") ";
        }
        os << ">" << std::endl;
    }

    return os;
}