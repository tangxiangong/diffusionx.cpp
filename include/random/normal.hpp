#ifndef NORMAL_HPP
#define NORMAL_HPP
#include <vector>
#include <format>
#include <type_traits>
#include "../error.hpp"
#include "random/utils.hpp"

using std::format;
using std::vector;


template<typename T = double> requires std::is_floating_point_v<T>
Result<vector<T> > randn(size_t n, T mean = 0, T stddev = 1) {
    if (stddev <= 0) {
        return Err(Error::InvalidArgument(format(
            "The standard deviation `stddev` must be positive, but got {}",
            stddev
        )));
    }
    auto sampler = [mean, stddev]() mutable -> T {
        thread_local static std::mt19937 gen = generator();
        std::normal_distribution<T> dist(mean, stddev);
        return dist(gen);
    };
    return Ok(parallel_generate<T>(n, sampler));
}

template<typename T = double> requires std::is_floating_point_v<T>
Result<vector<T> > randn(T mean = 0, T stddev = 1) {
    if (stddev <= 0) {
        return Err(Error::InvalidArgument(format(
            "The standard deviation `stddev` must be positive, but got {}",
            stddev
        )));
    }

    thread_local static std::mt19937 gen = generator();
    std::normal_distribution<T> dist(mean, stddev);
    return Ok(dist(gen));
}

template<typename T> requires std::is_floating_point_v<T>
class Normal {
    T m_mean = 0.0;
    T m_stddev = 1.0;

public:
    Normal() = default;

    Normal(T mean, T stddev) : m_mean(mean), m_stddev(stddev) {
        if (m_stddev <= 0) {
            throw std::invalid_argument(format(
                "The standard deviation `stddev` must be positive, but got {}",
                m_stddev
            ));
        }
    }

    [[nodiscard]] T get_mean() const {
        return m_mean;
    }

    [[nodiscard]] T get_stddev() const {
        return m_stddev;
    }

    [[nodiscard]] Result<std::vector<T> > sample(size_t n) const {
        return randn(n, m_mean, m_stddev);
    }

    Normal operator+(const Normal &rhs) const {
        double stddev = std::sqrt(m_stddev * m_stddev + rhs.get_stddev() * rhs.get_stddev());
        double mean = m_mean + rhs.get_mean();
        return Normal{mean, stddev};
    }

    Normal operator-() const {
        return Normal{-m_mean, m_stddev};
    }

    Normal operator-(const Normal &rhs) const {
        return *this + (-rhs);
    }

    friend Result<Normal> operator*(T a, const Normal &rhs) {
        if (a == 0.0) {
            return Err(Error::InvalidArgument("The scale number should not be zero."));
        }
        T mean = a * rhs.get_mean();
        T stddev = std::abs(a) * rhs.get_stddev();
        return Ok(Normal{mean, stddev});
    }

    friend Result<Normal> operator*(const Normal &lhs, T a) {
        return a * lhs;
    }

    friend Normal operator+(const Normal &lhs, T a) {
        T mean = a + lhs.get_mean();
        return Normal{mean, lhs.get_stddev()};
    }

    friend Normal operator+(T a, const Normal &rhs) {
        return rhs + a;
    }

    friend Normal operator-(const Normal &lhs, T a) {
        return lhs + (-a);
    }

    friend Normal operator-(T a, const Normal &rhs) {
        return a + (-rhs);
    }
};

#endif //NORMAL_HPP
