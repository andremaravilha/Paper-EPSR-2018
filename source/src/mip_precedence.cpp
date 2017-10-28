#include "mip_precedence.h"

#include <algorithm>
#include <cmath>

#include <gurobi_c++.h>

#include "heur_greedy.h"


std::tuple<orcs::Schedule, double> orcs::MIPPrecedence::solve(const Problem& problem,
        const cxxproperties::Properties* opt_input, cxxproperties::Properties* opt_output) {

    // Solver parameters
    bool verbose      = opt_input->get<bool>("verbose", false);
    int threads       = opt_input->get<int>("threads", 0);
    double time_limit = opt_input->get<double>("time-limit", GRB_INFINITY);
    double iterations_limit = opt_input->get<double>("iterations-limit", GRB_INFINITY);
    bool warm_start   = opt_input->get<double>("warm-start", false);
    bool solve_lr     = opt_input->get<double>("solve-relaxation", false);

    // Variable to keep the solution
    Schedule solution = create_empty_schedule(problem.m);

    // Get problem data
    auto n = problem.n;
    auto m = problem.m;
    auto s = problem.s;
    auto c = problem.c;
    auto p = problem.p;
    auto technology = problem.technology;
    auto action = problem.action;
    const auto& predecessors = problem.predecessors;

    // Add info about the dummy node (n+1)
    action.push_back(Action::UNKNOWN);
    technology.push_back(Technology::UNKNOWN);
    p.push_back(0);
    c.push_back(std::vector< std::vector<double> >(n + 2, std::vector<double>(m + 1, 0)));
    for (int i = 0; i <= n; ++i) {
        c[i].push_back(std::vector<double>(m + 1, 0));
    }

    // Compute the big-M value
    double M = 0.0;
    for (int j = 1; j <= n; ++j) {
        double max_c = 0.0;
        if (technology[j] != Technology::REMOTE) {
            for (int i = 0; i <= n; ++i) {
                if (i != j && technology[i] != Technology::REMOTE) {
                    for (int l = 1; l <= m; ++l) {
                        max_c = std::max(max_c, c[i][j][l]);
                    }
                }
            }
        }
        M += max_c + p[j];
    }

    // Solve the problem with Gurobi solver
    GRBEnv* env = nullptr;

    try {

        // Gurobi environment and model
        env = new GRBEnv();
        GRBModel model(*env);

        // Set some settings of Gurobi solver
        model.getEnv().set(GRB_IntParam_LogToConsole, (verbose ? 1 : 0));
        model.getEnv().set(GRB_IntParam_OutputFlag, (verbose ? 1 : 0));
        model.getEnv().set(GRB_IntParam_Threads, threads);
        model.getEnv().set(GRB_DoubleParam_TimeLimit, time_limit);
        model.getEnv().set(GRB_DoubleParam_NodeLimit, iterations_limit);

        // Allocate memory for decision variables
        GRBVar*** x = new GRBVar**[n + 2];
        for (int i = 0; i <= n + 1; ++i) {
            x[i] = new GRBVar*[n + 2];
            for (int j = 0; j <= n + 1; ++j) {
                x[i][j] = new GRBVar[m + 1];
            }
        }

        GRBVar* t = new GRBVar[n + 2];

        // Decision variables
        for (int i = 0; i <= n; ++i) {
            if (technology[i] != Technology::REMOTE) {
                for (int j = 1; j <= n + 1; ++j) {
                    if (j != i && technology[j] != Technology::REMOTE) {
                        for (int l = 1; l <= m; ++l) {
                            x[i][j][l] = model.addVar(0, 1, 0, GRB_BINARY);
                        }
                    }
                }
            }
        }

        for (int i = 0; i <= n + 1; ++i) {
            t[i] = model.addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS);
        }

        GRBVar T = model.addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS);

        model.update();

        // Warm start
        if (warm_start) {

            // Initially, make all variables equal zero
            for (int i = 0; i <= n; ++i) {
                if (technology[i] != Technology::REMOTE) {
                    for (int j = 1; j <= n + 1; ++j) {
                        if (j != i && technology[j] != Technology::REMOTE) {
                            for (int l = 1; l <= m; ++l) {
                                x[i][j][l].set(GRB_DoubleAttr_Start, 0.0);
                            }
                        }
                    }
                }
            }

            for (int i = 0; i <= n + 1; ++i) {
                t[i].set(GRB_DoubleAttr_Start, 0.0);
            }

            T.set(GRB_DoubleAttr_Start, 0.0);

            // Get a heuristic solution
            Greedy greedy;
            auto start = greedy.solve(problem);

            const Schedule& schedule = std::get<Schedule >(start);
            double makespan = std::get<double>(start);

            // Set initial value for the variables

            for (std::size_t idx = 0; idx < schedule[0].size(); ++idx) {
                int j = std::get<ATTR_SWITCH>(schedule[0][idx]);
                t[j].set(GRB_DoubleAttr_Start, std::get<ATTR_MOMENT>(schedule[0][idx]));
            }

            for (int l = 1; l <= m; ++l) {
                int prev = 0;
                for (std::size_t idx = 0; idx < schedule[l].size(); ++idx) {
                    int j = std::get<ATTR_SWITCH>(schedule[l][idx]);
                    x[prev][j][l].set(GRB_DoubleAttr_Start, 1.0);
                    t[j].set(GRB_DoubleAttr_Start, std::get<ATTR_MOMENT>(schedule[l][idx]));
                    prev = j;
                }
                x[prev][n + 1][l].set(GRB_DoubleAttr_Start, 1.0);
            }

            t[n + 1].set(GRB_DoubleAttr_Start, makespan);
            T.set(GRB_DoubleAttr_Start, makespan);
        }

        // Objective function
        GRBLinExpr objective = T;
        model.setObjective(objective, GRB_MINIMIZE);

        // Constraints 1
        for (int l = 1; l <= m; ++l) {
            GRBLinExpr expr = 0;
            for (int j = 1; j <= n + 1; ++j) {
                if (technology[j] != Technology::REMOTE) {
                    expr += x[0][j][l];
                }
            }
            model.addConstr(expr == 1);
        }

        // Constraints 2
        for (int l = 1; l <= m; ++l) {
            GRBLinExpr expr = 0;
            for (int i = 0; i <= n; ++i) {
                if (technology[i] != Technology::REMOTE) {
                    expr += x[i][n + 1][l];
                }
            }
            model.addConstr(expr == 1);
        }

        // Constraints 3
        for (int i = 1; i <= n; ++i) {
            if (technology[i] != Technology::REMOTE) {
                GRBLinExpr expr = 0;
                for (int j = 1; j <= n + 1; ++j) {
                    if (j != i && technology[j] != Technology::REMOTE) {
                        for (int l = 1; l <= m; ++l) {
                            expr += x[i][j][l];
                        }
                    }
                }
                model.addConstr(expr == 1);
            }
        }

        // Constraints 4
        for (int i = 1; i <= n; ++i) {
            if (technology[i] != Technology::REMOTE) {
                for (int l = 1; l <= m; ++l) {

                    GRBLinExpr expr = 0;

                    for (int j = 1; j <= n + 1; ++j) {
                        if (j != i && technology[j] != Technology::REMOTE) {
                            expr += x[i][j][l];
                        }
                    }

                    for (int j = 0; j <= n; ++j) {
                        if (j != i && technology[j] != Technology::REMOTE) {
                            expr -= x[j][i][l];
                        }
                    }

                    model.addConstr(expr == 0);
                }
            }
        }

        // Constraints 5
        for (int i = 0; i <= n; ++i) {
            if (technology[i] != Technology::REMOTE) {
                for (int j = 1; j <= n + 1; ++j) {
                    if (j != i && technology[j] != Technology::REMOTE) {
                        for (int l = 1; l <= m; ++l) {
                            model.addConstr(t[j] >= t[i] + p[i] + c[i][j][l] - M * (1 - x[i][j][l]));
                        }
                    }
                }
            }
        }

        // Constraints 6
        for (int j = 1; j <= n; ++j) {
            for (auto i : predecessors[j]) {
                model.addConstr(t[j] >= t[i] + p[i]);
            }
        }

        // Constraints 7
        for (int i = 1; i <= n; ++i) {
            model.addConstr(T >= t[i] + p[i]);
        }

        // Solve the model
        model.optimize();

        // Get the best solution found (if any)
        if (model.get(GRB_IntAttr_SolCount) > 0) {
            for (int j = 1; j <= n; ++j) {
                if (technology[j] != Technology::REMOTE) {
                    for (int i = 0; i <= n; ++i) {
                        if (i != j && technology[i] != Technology::REMOTE)  {
                            for (int l = 1; l <= m; ++l) {
                                if (x[i][j][l].get(GRB_DoubleAttr_X) > 0.5) {
                                    solution[l].push_back(Maneuver(j, t[j].get(GRB_DoubleAttr_X)));
                                }
                            }
                        }
                    }
                } else {
                    solution[0].push_back(Maneuver(j, t[j].get(GRB_DoubleAttr_X)));
                }
            }

            for (int l = 0; l <= m; ++l) {
                std::sort(solution[l].begin(), solution[l].end(),
                          [](const Maneuver& first, const Maneuver& second) -> bool {
                              return (std::get<ATTR_MOMENT>(first) < std::get<ATTR_MOMENT>(second));
                          });
            }
        }

        // Store optional output
        if (opt_output != nullptr) {

            // Status of the optimization process
            int status = model.get(GRB_IntAttr_Status);
            switch (status) {

                case GRB_OPTIMAL:
                    opt_output->add("Status", "OPTIMAL");
                    break;

                case GRB_INFEASIBLE:
                    opt_output->add("Status", "INFEASIBLE");
                    break;

                case GRB_UNBOUNDED:
                    opt_output->add("Status", "UNBOUNDED");
                    break;

                case GRB_INF_OR_UNBD:
                    opt_output->add("Status", "INF_OR_UNBD");
                    break;

                default:
                    if (model.get(GRB_IntAttr_SolCount) > 0) {
                        opt_output->add("Status", "SUBOPTIMAL");
                    } else {
                        opt_output->add("Status", "UNKNOWN");
                    }
            }

            // Objective function of the best solution found (if any)
            if (model.get(GRB_IntAttr_SolCount) > 0) {
                opt_output->add("MIP objective", model.get(GRB_DoubleAttr_ObjVal));
            }

            // Number of iterations (or MIP nodes)
            try {
                opt_output->add("Iterations", model.get(GRB_DoubleAttr_NodeCount));
            } catch (...) {
                // Do nothing
            }

            // MIP gap
            try {
                double mip_gap = model.get(GRB_DoubleAttr_MIPGap);
                if (std::abs(mip_gap - GRB_INFINITY) < 0.00001) {
                    opt_output->add("MIP gap", "Infinity");
                } else {
                    opt_output->add("MIP gap", mip_gap);
                }
            } catch (...) {
                // Do nothing
            }

            // Runtime
            opt_output->add("MIP runtime (s)", model.get(GRB_DoubleAttr_Runtime));

            // Solve the linear relaxation
            if (solve_lr) {

                // Reset Gurobi solver
                model.getEnv().set(GRB_IntParam_OutputFlag, 0);
                model.getEnv().set(GRB_DoubleParam_TimeLimit, GRB_INFINITY);
                model.reset();

                // Relax the integrality constraints
                for (int i = 0; i <= n; ++i) {
                    if (technology[i] != Technology::REMOTE) {
                        for (int j = 1; j <= n + 1; ++j) {
                            if (j != i && technology[j] != Technology::REMOTE) {
                                for (int l = 1; l <= m; ++l) {
                                    x[i][j][l].set(GRB_CharAttr_VType, GRB_CONTINUOUS);
                                }
                            }
                        }
                    }
                }

                // Solve the linear relaxation
                model.optimize();

                // Value of the objective function
                if (model.get(GRB_IntAttr_SolCount) > 0) {
                    opt_output->add("LP objective", model.get(GRB_DoubleAttr_ObjVal));
                }

                // Runtime
                opt_output->add("LP runtime (s)", model.get(GRB_DoubleAttr_Runtime));
            }
        }

        // Deallocate resources
        for (int i = 0; i <= n + 1; ++i) {
            for (int j = 0; j <= n + 1; ++j) {
                delete[] x[i][j];
                x[i][j] = nullptr;
            }
            delete[] x[i];
            x[i] = nullptr;
        }
        delete[] x;
        x = nullptr;

        delete[] t;
        t = nullptr;

    } catch (...) {

        // Deallocate resources
        if (env != nullptr) {
            delete env;
            env = nullptr;
        }

        // Re-throw the exception
        throw;
    }

    // Deallocate resources
    if (env != nullptr) {
        delete env;
        env = nullptr;
    }

    // Return the solution found
    return std::make_tuple(solution, problem.evaluate(solution));
};
