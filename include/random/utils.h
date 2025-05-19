#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <thread>
#include <future>
#include <cstddef>

template<typename T, typename F>
std::vector<T> parallel_sample(size_t n, F sampler) {
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    size_t block_size = n / num_threads;
    size_t remainder = n % num_threads;
    std::vector<std::future<std::vector<T>>> futures;
    for (unsigned int t = 0; t < num_threads; ++t) {
        size_t this_block = block_size + (t < remainder ? 1 : 0);
        futures.push_back(std::async(std::launch::async, [sampler, this_block]() mutable {
            std::vector<T> local_result;
            local_result.reserve(this_block);
            for (size_t i = 0; i < this_block; ++i) {
                local_result.push_back(sampler());
            }
            return local_result;
        }));
    }
    std::vector<T> result;
    result.reserve(n);
    for (auto& fut : futures) {
        auto part = fut.get();
        result.insert(result.end(), part.begin(), part.end());
    }
    return result;
}


#endif // UTILS_H 