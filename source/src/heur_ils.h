#ifndef MANEUVERS_SCHEDULING_ILS_H
#define MANEUVERS_SCHEDULING_ILS_H

#include <random>

#include "problem.h"
#include "algorithm.h"


namespace orcs {

    /**
     * This class implements an ILS-based heuristic for the maneuvers
     * scheduling problem in the restoration of electric power distribution
     * networks.
     */
    class ILS : public Algorithm {
    public:

        /**
         * This method implements the strategy for solving the problem.
         *
         * @param   problem
         *          The instance of the problem to solve.
         * @param   opt_input
         *          Optional input arguments. It is useful for setting
         *          parameters of the solver. It can be set to nullptr.
         * @param   opt_output
         *          Optional output arguments. It is useful for returning
         *          additional information about the solution proccess.
         *          It can be set to nullptr.
         * @return  A tuple of two elements, in which the first is the
         *          schedule and the second is the makespan.
         */
        std::tuple<Schedule, double> solve(const Problem& problem,
                const cxxproperties::Properties* opt_input = nullptr,
                cxxproperties::Properties* opt_output = nullptr);

    private:

        std::tuple<Schedule, double> perturb(const Problem& problem, const std::tuple<Schedule, double>& entry, std::mt19937& generator);

        void log_header(bool verbose = true);

        void log_footer(bool verbose = true);

        void log_start(double start, double time, bool verbose = true);

        void log_iteration(long iteration, double incumbent, double before_ls,
                double after_ls, double time, bool verbose = true);

    };

}


#endif
