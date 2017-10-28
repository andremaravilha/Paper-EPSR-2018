#ifndef MANEUVERS_SCHEDULING_PROBLEM_H
#define MANEUVERS_SCHEDULING_PROBLEM_H

#include <cstddef>
#include <tuple>
#include <vector>
#include <set>
#include <string>
#include <ostream>


namespace orcs {


    /**
     * Types of technologies used by switches.
     */
    enum class Technology {
        UNKNOWN,
        MANUAL,
        REMOTE
    };

    /**
     * Type of actions performed on a maneuver.
     */
    enum class Action {
        UNKNOWN,
        OPEN,
        CLOSE
    };

    /**
     * A maneuver. It is encoded as a tuple of two elements, in
     * which the 0-th element is ID of the switch and the 1-st
     * element is the moment in which the maneuver is performed.
     */
    using Maneuver = std::tuple<int, double>;

    /**
     * Aliases for accessing attributes of a maneuver.
     */
    static constexpr std::size_t ATTR_SWITCH = 0;
    static constexpr std::size_t ATTR_MOMENT = 1;

    /**
     * A solution for the problem. It is encoded as a vector ranging from
     * 0 to m schedules, in which the l-th position of the vector is the
     * schedule of team l.
     */
    using Schedule = std::vector< std::vector<Maneuver> >;

    /**
     * Create an empty schedule for m maintenance teams. The maintenance
     * teams are indexed from 1 to m. The index zero is used for
     * scheduling the remotely controlled switches.
     *
     * @param   m
     *          The number of maintenance teams available
     * @return  An empty schedule configured for m teams.
     */
    inline Schedule create_empty_schedule(int m) {
        return std::vector< std::vector<Maneuver> >(m+1, std::vector<Maneuver>());
    }

    /**
     * This class keeps the data of the maneuvers scheduling problem
     * in the restoration of electric power distribution networks.
     * The data of the problem is read from a file.
     */
    class Problem {

    public:

        int n;
        int m;
        int s;
        std::vector<Technology> technology;
        std::vector<Action> action;
        std::vector<int> stage;
        std::vector<double> p;
        std::vector< std::vector< std::vector<double> > > c;
        std::vector< std::vector<bool> > precedence_matrix;
        std::vector< std::set<int> > predecessors;
        std::vector< std::set<int> > successors;

        /**
         * Constructor.
         *
         * @param   filename
         *          Path to the file containing the data.
         */
        Problem(const std::string& filename);

        /**
         * Evaluate a schedule (compute the makespan) for this
         * problem.
         *
         * @param   schedule
         *          The schedule to evaluate.
         * @return  The makespan.
         */
        double evaluate(const Schedule &schedule) const;

        /**
         * Check if a schedule satisfies all constraints of
         * this problem.
         *
         * @param   schedule
         *          A schedule.
         * @param   msg
         *          If the solution is infeasible, this parameter will
         *          contain a description of the infeasibility. Otherwise,
         *          it will contain the content "Feasible solution.".
         * @return  True if the schedule satisfies all constraints,
         *          false otherwise.
         */
        bool is_feasible(const Schedule &schedule, std::string *msg = nullptr) const;

    };

}


/**
 * Overload the flow operator for printing a schedule to an output
 * stream.
 *
 * @param   os
 *          The output stream.
 * @param   schedule
 *          The schedule to print.
 * @return  A reference to the output stream.
 */
std::ostream& operator<<(std::ostream& os, orcs::Schedule schedule);


#endif
