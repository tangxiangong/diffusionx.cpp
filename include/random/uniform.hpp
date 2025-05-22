#ifndef UNIFORM_H
#define UNIFORM_H
#include <vector>
#include <format>
#include <type_traits>
#include "../error.h"
#include "random/utils.hpp"

using std::vector;
using std::format;

template<typename T = double> requires std::is_floating_point_v<T> || std::is_integral_v<T>
Result<vector<T> > rand(size_t n = 1, T a = 0, T b = 1) {
    if (a > b) {
        return Err(Error::InvalidArgument(format(
            "The lower bound `a` must be less than the upper bound `b`, but got {} > {}",
            a, b
        )));
    }

    if (std::is_integral_v<T>) {
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

#endif //UNIFORM_H
