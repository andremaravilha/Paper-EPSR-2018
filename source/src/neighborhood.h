#ifndef MANEUVERS_SCHEDULING_NEIGHBORHOOD_H
#define MANEUVERS_SCHEDULING_NEIGHBORHOOD_H

#include <random>

#include "algorithm.h"
#include "problem.h"


namespace orcs {

    /**
     * Base class that defines the interface implemented by all neighborhoods.
     */
    class Neighborhood {

    public:

        /**
         * Return the best neighbor of the given entry.
         *
         * @param   problem
         *          Instance of the problem being optimized.
         * @param   entry
         *          The start solution to perform the local search.
         *
         * @return  A tuple of two elements, in which the first is the
         *          schedule and the second is the makespan.
         */
        virtual std::tuple<orcs::Schedule, double> best_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry) = 0;

        /**
         * Return the first neighbor that improves the objective function.
         *
         * @param   problem
         *          Instance of the problem being optimized.
         * @param   entry
         *          The start solution to perform the local search.
         * @param   generator
         *          The random number generator.
         *
         * @return  A tuple of two elements, in which the first is the
         *          schedule and the second is the makespan.
         */
        virtual std::tuple<orcs::Schedule, double> first_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry,
                std::mt19937& generator) = 0;

        /**
         * Return the first neighbor that improves the objective function.
         *
         * @param   problem
         *          Instance of the problem being optimized.
         * @param   entry
         *          The start solution to perform the local search.
         * @param   feasible_only
         *          If false, only feasible solutions can be returned. If false,
         *          infeasible solutions can be returned.
         * @param   generator
         *          The random number generator.
         *
         * @return  A tuple of two elements, in which the first is the
         *          schedule and the second is the makespan.
         */
        virtual std::tuple<orcs::Schedule, double> shake(const Problem& problem,
                const std::tuple<Schedule, double>& entry, bool feasible_only,
                std::mt19937& generator) = 0;


    };


    /**
     * Shift neighborhood.
     */
    class Shift : public Neighborhood {

        std::tuple<orcs::Schedule, double> best_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry) override;

        std::tuple<orcs::Schedule, double> first_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry,
                std::mt19937& generator) override;

        std::tuple<orcs::Schedule, double> shake(const Problem& problem,
                const std::tuple<Schedule, double>& entry, bool feasible_only,
                std::mt19937& generator) override;

    };

    /**
     * Reassignment neighborhood.
     */
    class Reassignement : public Neighborhood {

        std::tuple<orcs::Schedule, double> best_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry) override;

        std::tuple<orcs::Schedule, double> first_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry,
                std::mt19937& generator) override;

        std::tuple<orcs::Schedule, double> shake(const Problem& problem,
                const std::tuple<Schedule, double>& entry, bool feasible_only,
                std::mt19937& generator) override;

    };

    /**
     * Swap neighborhood.
     */
    class Swap : public Neighborhood {

        std::tuple<orcs::Schedule, double> best_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry) override;

        std::tuple<orcs::Schedule, double> first_improvement(const Problem& problem,
                const std::tuple<Schedule, double>& entry,
                std::mt19937& generator) override;

        std::tuple<orcs::Schedule, double> shake(const Problem& problem,
                const std::tuple<Schedule, double>& entry, bool feasible_only,
                std::mt19937& generator) override;

    };

}


#endif
