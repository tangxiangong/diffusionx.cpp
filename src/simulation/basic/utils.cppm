/**
 * @file utils.cppm
 * @brief Utility functions for basic simulation
 *
 * This module provides utility functions for basic simulation, including
 * parallel Monte Carlo simulation and statistical analysis.
 */

module;

#include <algorithm>
#include <format>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef DIFFUSIONX_USE_TBB
#include <tbb/tbb.h>
#endif

export module diffusionx.simulation.basic.utils;

import diffusionx.error;

using std::format;
using std::vector;

export template <typename T>
concept Float = std::is_floating_point_v<T>;

export template <typename T>
concept UnsignedInt = std::is_unsigned_v<T>;

export template <typename T>
concept Real = std::is_floating_point_v<T> || std::is_integral_v<T>;

export using vec_pair = std::pair<vector<double>, vector<double>>;

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
export template <typename F, Real R = double>
    requires(std::invocable<F> && std::same_as<std::invoke_result_t<F>, R>)
auto parallel_monte_carlo(size_t particles, F func) -> Result<double> {
    if (particles == 0) {
        return Err(Error::InvalidArgument(
            "The number of particles must be greater than 0"));
    }

#ifdef DIFFUSIONX_USE_TBB
    R total_sum = tbb::parallel_reduce(
        tbb::blocked_range<size_t>(0, particles), static_cast<R>(0),
        [func](const tbb::blocked_range<size_t> &range, R init) {
            auto local_func = func;
            for (size_t i = range.begin(); i < range.end(); ++i) {
                init += local_func();
            }
            return init;
        },
        std::plus<R>());
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

    vector<R> partial_results(num_threads, 0);
    size_t chunk_size = (particles + num_threads - 1) / num_threads;

    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min(start + chunk_size, particles);
        threads.emplace_back([&partial_results, i, start, end, func]() {
            R local_sum = 0;
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

    R total_sum = 0;
    for (R partial : partial_results) {
        total_sum += partial;
    }
#endif

    return static_cast<double>(total_sum) / static_cast<double>(particles);
}

export template <Float T>
Result<void> check_duration_time_step(T duration, T time_step) {
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
