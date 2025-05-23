#ifndef POISSON_HPP
#define POISSON_HPP

#include <vector>
#include <format>
#include <random>
#include "../error.hpp"
#include "random/utils.hpp"

using std::vector;
using std::format;

template<typename T = unsigned int> requires std::is_unsigned_v<T>
Result<vector<T> > rand_poisson(size_t n, double rate = 1.0) {
    if (rate <= 0) {
        return Err(Error::InvalidArgument(format(
            "The rate `rate` must be positive, but got {}",
            rate
        )));
    }
    auto sampler = [rate]() mutable {
        thread_local static std::mt19937 gen = generator();
        std::poisson_distribution<T> dist(rate);
        return dist(gen);
    };
    return Ok(parallel_generate<T>(n, sampler));
}

template<typename T = unsigned int> requires std::is_unsigned_v<T>
Result<vector<T> > rand_poisson(double rate = 1.0) {
    if (rate <= 0) {
        return Err(Error::InvalidArgument(format(
            "The rate `rate` must be positive, but got {}",
            rate
        )));
    }

    thread_local std::mt19937 gen = generator();
    std::poisson_distribution<T> dist(rate);
    return Ok(dist(gen));
}


class Poisson {
    double m_rate = 1.0;

public:
    Poisson() = default;

    explicit Poisson(double rate) : m_rate(rate) {
        if (m_rate <= 0) {
            throw std::invalid_argument(format(
                "The rate parameter `rate` must be positive, but got {}",
                m_rate
            ));
        }
    }

    [[nodiscard]] double get_rate() const {
        return m_rate;
    }

    template<typename T = unsigned int> requires std::is_unsigned_v<T>
    [[nodiscard]] Result<vector<T> > sample(size_t n) const {
        return rand_poisson<T>(n, m_rate);
    }
};

#endif //POISSON_HPP
