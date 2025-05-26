module;

#include <vector>
#include <thread>
#include <random>
#include <algorithm>

export module diffusionx.random.utils;

export inline auto generator() -> std::mt19937 {
    std::random_device rd;
    std::seed_seq ssq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
    return std::mt19937(ssq);
}

export template<typename T, typename F>
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
