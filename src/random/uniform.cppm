module;

#include <format>
#include <random>
#include <type_traits>
#include <vector>

export module diffusionx.random.uniform;

import diffusionx.error;
import diffusionx.random.utils;

using std::format;
using std::vector;

/**
 * @brief Generates a vector of uniformly distributed random values
 * @tparam T The numeric type (floating-point or integral)
 * @param n The number of values to generate
 * @param a The lower bound of the distribution (inclusive for integers, inclusive for floats)
 * @param b The upper bound of the distribution (inclusive for integers, exclusive for floats)
 * @return Result containing a vector of n uniformly distributed values, or an Error
 * 
 * This function generates n random values uniformly distributed between a and b.
 * For integral types, both bounds are inclusive. For floating-point types,
 * the lower bound is inclusive and the upper bound is exclusive.
 * 
 * @note Uses parallel generation for improved performance
 * @note Each thread uses its own thread-local generator for thread safety
 */
export template<typename T = double>
    requires std::is_floating_point_v<T> || std::is_integral_v<T>
auto rand(size_t n, T a = 0, T b = 1) -> Result<vector<T> > {
    if (a > b) {
        return Err(Error::InvalidArgument(
            format("The lower bound `a` must be less than "
                   "the upper bound `b`, but got {} > {}",
                   a, b)));
    }

    if constexpr (std::is_integral_v<T>) {
        auto sampler = [a, b]() mutable {
            thread_local std::mt19937 gen = generator();
            std::uniform_int_distribution<T> dist{a, b};
            return dist(gen);
        };
        return Ok(parallel_generate<T>(n, sampler));
    } else {
        auto sampler = [a, b]() mutable {
            thread_local std::mt19937 gen = generator();
            std::uniform_real_distribution<T> dist{a, b};
            return dist(gen);
        };
        return Ok(parallel_generate<T>(n, sampler));
    }
}

/**
 * @brief Generates a single uniformly distributed random value
 * @tparam T The numeric type (floating-point or integral)
 * @param a The lower bound of the distribution (inclusive for integers, inclusive for floats)
 * @param b The upper bound of the distribution (inclusive for integers, exclusive for floats)
 * @return Result containing a uniformly distributed value, or an Error
 * 
 * This function generates a single random value uniformly distributed between a and b.
 * For integral types, both bounds are inclusive. For floating-point types,
 * the lower bound is inclusive and the upper bound is exclusive.
 * 
 * @note Uses thread-local generator for thread safety
 */
export template<typename T = double>
    requires std::is_floating_point_v<T> || std::is_integral_v<T>
auto rand(T a = 0, T b = 1) -> Result<T> {
    if (a > b) {
        return Err(Error::InvalidArgument(
            format("The lower bound `a` must be less than "
                   "the upper bound `b`, but got {} > {}",
                   a, b)));
    }

    if constexpr (std::is_integral_v<T>) {
        thread_local std::mt19937 gen = generator();
        std::uniform_int_distribution<T> dist{a, b};
        return Ok(dist(gen));
    } else {
        thread_local std::mt19937 gen = generator();
        std::uniform_real_distribution<T> dist{a, b};
        return Ok(dist(gen));
    }
}
