#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <thread>
#include <future>
#include <cstddef>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

template<typename T, typename F>
std::vector<T> parallel_generate(size_t n, F sampler) {
    std::vector<T> result(n); // Pre-allocate the entire result vector
    if (n == 0) {
        return result;
    }

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, n),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                result[i] = sampler();
            }
        }
    );

    return result;
}


#endif // UTILS_H
