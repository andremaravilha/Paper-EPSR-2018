#ifndef MANEUVERS_SCHEDULING_UTILS_H
#define MANEUVERS_SCHEDULING_UTILS_H

#include <cstddef>
#include <random>
#include <string>
#include <tuple>
#include <type_traits>

#include "problem.h"


namespace orcs {

    namespace utils {

        /**
         * The threshold used when compared equality of floating point values.
         */
        constexpr double THRESHOLD = 1e-5;

        /**
         * Compare two numbers. This function considers a threshold value
         * to compare the numbers. If the difference between they are
         * less then THRESHOLD constant, then the numbers are considered
         * as equals.
         *
         * @param   first
         *          A number.
         * @param   second
         *          A number.
         *
         * @return  Return -1 if first is less then second, 0 if both numbers
         *          are equal, 1 if less is greater than second.
         */
        int compare(double first, double second);

        /**
         * Compare equality between two numbers. This function considers a
         * threshold value to compare the numbers. If the difference between
         * they are less then THRESHOLD constant, then the numbers are considered
         * as equals.
         *
         * @param   first
         *          A number.
         * @param   second
         *          A number.
         *
         * @return  True if the numbers are equal, false otherwise.
         */
        bool is_equal(double first, double second);

        /**
         * Check if the first number is greater than the second one. This function
         * considers a threshold value to compare the numbers. If the difference
         * between they are less then THRESHOLD constant, then the numbers are
         * considered as equals.
         *
         * @param   first
         *          A number.
         * @param   second
         *          A number.
         *
         * @return  True if first is greater than second, false otherwise.
         */
        bool is_greater(double first, double second);

        /**
         * Check if the first number is lower than the second one. This function
         * considers a threshold value to compare the numbers. If the difference
         * between they are less then THRESHOLD constant, then the numbers are
         * considered as equals.
         *
         * @param   first
         *          A number.
         * @param   second
         *          A number.
         *
         * @return  True if first is lower than second, false otherwise.
         */
        bool is_lower(double first, double second);

        /**
         * Check if the first number is greater of equal to the second one. This
         * function considers a threshold value to compare the numbers. If the
         * difference between they are less then THRESHOLD constant, then the
         * numbers are considered as equals.
         *
         * @param   first
         *          A number.
         * @param   second
         *          A number.
         *
         * @return  True if first is greater or equal to the second number,
         *          false otherwise.
         */
        bool is_greater_equal(double first, double second);

        /**
         * Check if the first number is lower of equal to the second one. This
         * function considers a threshold value to compare the numbers. If the
         * difference between they are less then THRESHOLD constant, then the
         * numbers are considered as equals.
         *
         * @param   first
         *          A number.
         * @param   second
         *          A number.
         *
         * @return  True if first is lower or equal to the second number,
         *          false otherwise.
         */
        bool is_lower_equal(double first, double second);

        /**
         * String formatter.
         *
         * @tparam  T
         * @param   str
         * @param   args
         * @return
         */
        template <class... T>
        std::string format(const std::string& str, T... args);

        /**
         * This function randomly chooses a element from the container values accordingly to their
         * respective weights.
         *
         * @tparam  TObject
         * @tparam  TContainer1
         * @tparam  TContainer2
         * @tparam  TRandom
         * @param   values
         * @param   weights
         * @param   generator
         * @return
         */
        template <class TObject, template<class, class...> class TContainer1, template<class, class...> class TContainer2, class TRandom>
        std::tuple<TObject, std::size_t> choose(const TContainer1<TObject>& values, const TContainer2<double>& weights, TRandom& generator);

        /**
         * Evaluate a scheduling and updates the entry with the result of the evaluation. Besides,
         * it returns whether the scheduling is feasible or not.
         *
         * @param   problem
         * @param   entry
         * @return
         */
        bool evaluate(const Problem& problem, std::tuple<Schedule, double>& entry);

    }

}


/*
 * Function definition.
 */

template <class... T>
std::string orcs::utils::format(const std::string& str, T... args) {

    // Define the buffer size
    std::size_t buffer_size = std::max(static_cast<std::size_t>(1024),
                                       static_cast<std::size_t>(2 * str.length()));

    // Create the formmatted string
    char* buffer = new char[buffer_size];
    sprintf(buffer, str.c_str(), args...);

    // Return the formmatted string as a std::string object
    return std::string(buffer);
}

template <class TObject, template<class, class...> class TContainer1, template<class, class...> class TContainer2, class TRandom>
std::tuple<TObject, std::size_t> orcs::utils::choose(const TContainer1<TObject>& values, const TContainer2<double>& weights, TRandom& generator) {

    // Sum all weights
    double sum = 0.0;
    for (const auto& w : weights) {
        sum += w;
    }

    // Get a random number
    double random_value = (generator() / (double) generator.max()) * sum;

    // Choose a value
    std::size_t index = 0;
    auto iter_values = values.begin();
    auto iter_weights = weights.begin();

    double aux = 0.0;
    while (iter_values != values.end()) {
        if (aux + *iter_weights >= random_value) {
            break;
        } else {
            aux += *iter_weights;
            ++index;
            ++iter_values;
            ++iter_weights;
        }
    }

    // Return the chosen value
    return std::make_tuple(*iter_values, index);
}

#endif
