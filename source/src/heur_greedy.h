#ifndef MANEUVERS_SCHEDULING_GREEDY_H
#define MANEUVERS_SCHEDULING_GREEDY_H

#include "problem.h"
#include "algorithm.h"


namespace orcs {

    /**
     * This class implements a greedy heuristic for the maneuvers scheduling
     * problem in the restoration of electric power distribution networks.
     * This greedy heuristic is based on the earliest start time (EST).
     */
    class Greedy : public Algorithm {

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
        std::tuple<Schedule, double> solve(const Problem &problem,
                const cxxproperties::Properties *opt_input = nullptr,
                cxxproperties::Properties *opt_output = nullptr);

    };

}

#endif
