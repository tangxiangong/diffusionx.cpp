module;

#include <random>
#include <thread>
#include <utility>
#include <vector>

export module diffusionx.random.utils;

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
 * multiple threads for improved performance. The number of threads used
 * is determined by std::thread::hardware_concurrency(), with fallbacks
 * for edge cases.
 * 
 * @note The sampler function should be thread-safe or use thread-local storage
 * @note For small values of n, fewer threads may be used for efficiency
 */
export template<typename T, typename F>
auto parallel_generate(size_t n, F sampler) -> vector<T> {
    vector<T> result(n);
    if (n == 0) {
        return result;
    }

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 1; // Fallback if hardware_concurrency is not available
        // or returns 0
    }
    if (std::cmp_less(n , num_threads)) {
        // If n is small, no need for many threads
        num_threads = static_cast<unsigned int>(n);
    }

    vector<std::thread> threads;
    threads.reserve(num_threads);

    size_t chunk_size = n / num_threads;
    size_t remainder = n % num_threads;

    for (unsigned int i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = start + chunk_size;
        if (i == num_threads - 1) {
            end += remainder; // Last thread handles the remainder
        }

        threads.emplace_back([&result, start, end, sampler]() mutable {
            for (size_t j = start; j < end; ++j) {
                result[j] = sampler();
            }
        });
    }

    for (auto &thread: threads) {
        thread.join();
    }

    return result;
}
