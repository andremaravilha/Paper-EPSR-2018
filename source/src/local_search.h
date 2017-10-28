#ifndef MANEUVERS_SCHEDULING_LOCAL_SEARCH_H
#define MANEUVERS_SCHEDULING_LOCAL_SEARCH_H

#include <list>
#include <random>

#include "algorithm.h"
#include "neighborhood.h"
#include "problem.h"


namespace orcs {

    /**
     * Class that implements a basic local search.
     */
    class LocalSearch {

    public:

        /**
         * Perform the local search according to the best improvement rule.
         *
         * @param   problem
         *          Instance of the problem being optimized.
         * @param   entry
         *          The start solution to perform the local search.
         * @param   neighborhood
         *          The neighborhood used to perform the local search.
         *
         * @return  A tuple of two elements, in which the first is the
         *          schedule and the second is the makespan.
         */
        static std::tuple<orcs::Schedule, double> best_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry, Neighborhood& neighborhood);

        /**
         * Perform the local search according to the first improvement rule.
         *
         * @param   problem
         *          Instance of the problem being optimized.
         * @param   entry
         *          The start solution to perform the local search.
         * @param   neighborhood
         *          The neighborhood used to perform the local search.
         * @param   generator
         *          The random number generator.
         *
         * @return  A tuple of two elements, in which the first is the
         *          schedule and the second is the makespan.
         */
        static std::tuple<orcs::Schedule, double> first_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry, Neighborhood& neighborhood,
                std::mt19937& generator);

        /**
         * Perform the local search according to variable neighborhood search (VND).
         * This method uses the best improvement rule.
         *
         * @param   problem
         *          Instance of the problem being optimized.
         * @param   entry
         *          The start solution to perform the local search.
         * @param   neighborhoods
         *          The list of neighborhood used to perform the VND.
         *
         * @return  A tuple of two elements, in which the first is the
         *          schedule and the second is the makespan.
         */
        static std::tuple<orcs::Schedule, double> best_improvement_vnd(const Problem& problem,
                const std::tuple<Schedule, double>& entry, std::list<Neighborhood*>& neighborhoods);

        /**
         * Perform the local search according to variable neighborhood search (VND).
         * This method uses the best improvement rule.
         *
         * @param   problem
         *          Instance of the problem being optimized.
         * @param   entry
         *          The start solution to perform the local search.
         * @param   neighborhood
         *          The list of neighborhood used to perform the VND.
         * @param   generator
         *          The random number generator.
         *
         * @return  A tuple of two elements, in which the first is the
         *          schedule and the second is the makespan.
         */
        static std::tuple<orcs::Schedule, double> first_improvement_vnd(const Problem& problem,
                const std::tuple<Schedule, double>& entry, std::list<Neighborhood*>& neighborhoods,
                std::mt19937& generator);
    };

}


#endif
