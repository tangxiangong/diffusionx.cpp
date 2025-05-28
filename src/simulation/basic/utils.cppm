/**
 * @file utils.cppm
 * @brief Utility functions for basic simulation
 * 
 * This module provides utility functions for basic simulation, including
 * parallel Monte Carlo simulation and statistical analysis.
 */

module;

#include <utility>
#include <vector>
#include <thread>
#include <string>
#include <optional>

export module diffusionx.simulation.basic.utils;

import diffusionx.error;

using std::vector;

/**
 * @brief Type alias for a pair of vectors representing time and position data
 * 
 * This type is commonly used to represent simulation trajectories where the first
 * vector contains time points and the second vector contains corresponding positions.
 */
export using vec_pair = std::pair<vector<double>, vector<double> >;

/**
 * @brief Type alias for a pair of doubles representing a domain or interval
 * 
 * This type is used to represent intervals, domains, or ranges in simulations.
 */
export using double_pair = std::pair<double, double>;


/**
 * @brief Performs parallel Monte Carlo simulation for statistical computations
 * @tparam ProcessType The type of the stochastic process
 * @tparam ComputeFunc The type of the computation function
 * @param particles The number of Monte Carlo samples
 * @param process Reference to the stochastic process
 * @param compute_func Function that computes a value from a single trajectory
 * @return Result containing the averaged result, or an Error
 * 
 * This function distributes Monte Carlo simulations across multiple threads
 * for improved performance. Each thread runs a subset of simulations and
 * the results are averaged.
 */
export template<typename ProcessType, typename ComputeFunc>
auto parallel_monte_carlo(size_t particles, ProcessType &process, ComputeFunc compute_func) -> Result<double> {
    if (particles == 0) {
        return Err(Error::InvalidArgument("The number of particles must be greater than 0"));
    }

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 1;
    }
    if (std::cmp_less(particles, num_threads)) {
        num_threads = static_cast<unsigned int>(particles);
    }

    vector<std::thread> threads;
    threads.reserve(num_threads);

    vector<double> partial_results(num_threads, 0.0);
    vector<bool> success_flags(num_threads, true);
    vector<std::optional<Error> > errors(num_threads);

    size_t chunk_size = particles / num_threads;
    size_t remainder = particles % num_threads;

    for (unsigned int i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = start + chunk_size;
        if (i == num_threads - 1) {
            end += remainder;
        }

        threads.emplace_back([&, i, start, end]() {
            double local_sum = 0.0;
            for (size_t j = start; j < end; ++j) {
                auto result = compute_func(process);
                if (!result.has_value()) {
                    success_flags[i] = false;
                    errors[i] = result.error();
                    return;
                }
                local_sum += result.value();
            }
            partial_results[i] = local_sum;
        });
    }

    for (auto &thread: threads) {
        thread.join();
    }

    // Check for errors
    for (unsigned int i = 0; i < num_threads; ++i) {
        if (!success_flags[i]) {
            return Err(errors[i].value());
        }
    }

    // Sum up partial results
    double total_sum = 0.0;
    for (double partial: partial_results) {
        total_sum += partial;
    }

    return total_sum / particles;
}
