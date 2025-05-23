#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <thread>
#include <random>
#include <algorithm>

inline auto generator() -> std::mt19937 {
    std::random_device rd;
    std::seed_seq ssq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
    return std::mt19937(ssq);
}

template<typename T, typename F>
std::vector<T> parallel_generate(size_t n, F sampler) {
    std::vector<T> result(n);
    if (n == 0) {
        return result;
    }

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 1; // Fallback if hardware_concurrency is not available or returns 0
    }
    if (n < num_threads) {
        // If n is small, no need for many threads
        num_threads = static_cast<unsigned int>(n);
    }


    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    size_t block_size = (n + num_threads - 1) / num_threads; // Calculate block size, ceiling division

    for (unsigned int i = 0; i < num_threads; ++i) {
        size_t start_index = i * block_size;
        size_t end_index = std::min(start_index + block_size, n);

        if (start_index < end_index) {
            // Ensure there's work to do
            threads.emplace_back([&result, sampler, start_index, end_index]() mutable {
                for (size_t j = start_index; j < end_index; ++j) {
                    result[j] = sampler();
                }
            });
        }
    }

    for (std::thread &t: threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return result;
}

#endif // UTILS_HPP
