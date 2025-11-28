/**
 * @file utils.cppm
 * @brief Utility functions for basic simulation
 *
 * This module provides utility functions for basic simulation, including
 * parallel Monte Carlo simulation and statistical analysis.
 */

module;

#include <format>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#ifdef DIFFUSIONX_USE_TBB
#include <tbb/tbb.h>
#endif

export module diffusionx.simulation.basic.utils;

import diffusionx.error;

using std::format;
using std::vector;

/**
 * @brief Type alias for a pair of vectors representing time and position data
 *
 * This type is commonly used to represent simulation trajectories where the
 * first vector contains time points and the second vector contains
 * corresponding positions.
 */
export using vec_pair = std::pair<vector<double>, vector<double>>;

/**
 * @brief Type alias for a pair of doubles representing a domain or interval
 *
 * This type is used to represent intervals, domains, or ranges in simulations.
 */
export using double_pair = std::pair<double, double>;

/**
 * @brief Performs parallel Monte Carlo simulation for statistical computations
 * @tparam F The type of the computation function
 * @param particles The number of Monte Carlo samples
 * @param func Function that computes a value from a single simulation
 * @return Result containing the averaged result, or an Error
 *
 * This function distributes Monte Carlo simulations across multiple threads
 * for improved performance. Uses TBB if DIFFUSIONX_USE_TBB is defined,
 * otherwise uses std::thread.
 */
export template <typename F>
    requires(std::invocable<F> && std::same_as<std::invoke_result_t<F>, double>)
auto parallel_monte_carlo(size_t particles, F func) -> Result<double> {
    if (particles == 0) {
        return Err(Error::InvalidArgument(
            "The number of particles must be greater than 0"));
    }

#ifdef DIFFUSIONX_USE_TBB
    double total_sum = tbb::parallel_reduce(
        tbb::blocked_range<size_t>(0, particles), 0.0,
        [func](const tbb::blocked_range<size_t> &range, double init) {
            auto local_func = func;
            for (size_t i = range.begin(); i < range.end(); ++i) {
                init += local_func();
            }
            return init;
        },
        std::plus<double>());
#else
    size_t num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 1;
    }
    if (std::cmp_less(particles, num_threads)) {
        num_threads = particles;
    }

    vector<std::thread> threads;
    threads.reserve(num_threads);

    vector<double> partial_results(num_threads, 0.0);
    size_t chunk_size = (particles + num_threads - 1) / num_threads;

    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min(start + chunk_size, particles);
        threads.emplace_back([&partial_results, i, start, end, func]() mutable {
            double local_sum = 0.0;
            for (size_t j = start; j < end; ++j) {
                local_sum += func();
            }
            partial_results[i] = local_sum;
        });
    }

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    double total_sum = 0.0;
    for (double partial : partial_results) {
        total_sum += partial;
    }
#endif

    return total_sum / static_cast<double>(particles);
}

export Result<void> check_duration_time_step(double duration,
                                             double time_step) {
    if (duration <= 0.0) {
        return Err(Error::InvalidArgument(
            format("The `duration` must be positive, got {}", duration)));
    }

    if (time_step <= 0.0) {
        return Err(Error::InvalidArgument(
            format("The `time_step` must be positive, got `{}`", time_step)));
    }

    if (time_step > duration) {
        return Err(Error::InvalidArgument(
            format("The `time_step` must be less than or equal to the "
                   "`duration`, got `{}` > `{}`",
                   time_step, duration)));
    }

    return Ok();
}
