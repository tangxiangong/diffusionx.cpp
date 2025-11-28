module;

#include <concepts>
#include <random>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef DIFFUSIONX_USE_TBB
#include <tbb/tbb.h>
#endif

export module diffusionx.random.utils;

export template <typename T>
concept Float = std::is_floating_point_v<T>;

export template <typename T>
concept UnsignedInt = std::is_unsigned_v<T>;

export template <typename T>
concept Real = std::is_floating_point_v<T> || std::is_integral_v<T>;

using std::vector;

/**
 * @brief Creates a random number generator seeded with a random device
 * @return A properly seeded Mersenne Twister generator
 *
 * This function creates a new std::mt19937 generator seeded with a hardware
 * random device. Each call returns a new generator instance.
 */
export inline auto generator() -> std::mt19937 {
    std::random_device rd;
    return std::mt19937(rd());
}

/**
 * @brief Generates random values in parallel using multiple threads
 * @tparam T The type of values to generate
 * @tparam F The type of the sampling function
 * @param n The number of values to generate
 * @param sampler A callable that generates a single random value of type T
 * @return A vector containing n randomly generated values
 *
 * This function distributes the work of generating random values across
 * multiple threads for improved performance. Uses TBB if DIFFUSIONX_USE_TBB
 * is defined, otherwise uses std::thread.
 *
 * @note The sampler function should be thread-safe or use thread-local storage
 */
export template <typename T, typename F>
    requires(std::invocable<F> && std::same_as<std::invoke_result_t<F>, T>)
auto parallel_generate(size_t n, F sampler) -> vector<T> {
    vector<T> result(n);
    if (n == 0) {
        return result;
    }

#ifdef DIFFUSIONX_USE_TBB
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, n),
        [&result, sampler](const tbb::blocked_range<size_t> &range) {
            auto local_sampler = sampler;
            for (size_t i = range.begin(); i < range.end(); ++i) {
                result[i] = local_sampler();
            }
        });
#else
    size_t num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 1;
    }
    if (std::cmp_less(n, num_threads)) {
        num_threads = n;
    }

    vector<std::thread> threads;
    threads.reserve(num_threads);

    size_t chunk_size = (n + num_threads - 1) / num_threads;

    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min(start + chunk_size, n);
        threads.emplace_back([&result, start, end, sampler]() mutable {
            for (size_t j = start; j < end; ++j) {
                result[j] = sampler();
            }
        });
    }

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
#endif

    return result;
}
