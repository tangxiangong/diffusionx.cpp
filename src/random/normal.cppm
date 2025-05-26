module;

#include <format>
#include <random>
#include <type_traits>
#include <vector>

export module diffusionx.random.normal;

import diffusionx.error;
import diffusionx.random.utils;

using std::format;
using std::vector;

export template <typename T = double>
    requires std::is_floating_point_v<T>
auto randn(size_t n, T mean = 0, T stddev = 1) -> Result<vector<T>> {
    if (stddev <= 0) {
        return Err(Error::InvalidArgument(format(
            "The standard deviation `stddev` must be positive, but got {}",
            stddev)));
    }
    auto sampler = [mean, stddev]() mutable -> T {
        thread_local static std::mt19937 gen = generator();
        std::normal_distribution<T> dist(mean, stddev);
        return dist(gen);
    };
    return Ok(parallel_generate<T>(n, sampler));
}

export template <typename T = double>
    requires std::is_floating_point_v<T>
auto randn(T mean = 0, T stddev = 1) -> Result<T> {
    if (stddev <= 0) {
        return Err(Error::InvalidArgument(format(
            "The standard deviation `stddev` must be positive, but got {}",
            stddev)));
    }
    thread_local static std::mt19937 gen = generator();
    std::normal_distribution<T> dist(mean, stddev);
    return Ok(dist(gen));
}

export template <typename T>
    requires std::is_floating_point_v<T>
class Normal {
    T m_mean = 0.0;
    T m_stddev = 1.0;

   public:
    Normal() = default;

    Normal(T mean, T stddev) : m_mean(mean), m_stddev(stddev) {
        if (m_stddev <= 0) {
            throw std::invalid_argument(format(
                "The standard deviation `stddev` must be positive, but got {}",
                m_stddev));
        }
    }

    [[nodiscard]] auto get_mean() const -> T { return m_mean; }

    [[nodiscard]] auto get_stddev() const -> T { return m_stddev; }

    [[nodiscard]] auto sample(size_t n) const -> Result<vector<T>> {
        return randn(n, m_mean, m_stddev);
    }

    auto operator+(const Normal &rhs) const -> Normal {
        double stddev = std::sqrt(m_stddev * m_stddev +
                                  rhs.get_stddev() * rhs.get_stddev());
        double mean = m_mean + rhs.get_mean();
        return Normal{mean, stddev};
    }

    auto operator-() const -> Normal { return Normal{-m_mean, m_stddev}; }

    auto operator-(const Normal &rhs) const -> Normal { return *this + (-rhs); }

    friend auto operator*(T a, const Normal &rhs) -> Normal {
        if (a == 0.0) {
            return Err(
                Error::InvalidArgument("The scale number should not be zero."));
        }
        T mean = a * rhs.get_mean();
        T stddev = std::abs(a) * rhs.get_stddev();
        return Ok(Normal{mean, stddev});
    }

    friend auto operator*(const Normal &lhs, T a) -> Normal { return a * lhs; }

    friend auto operator+(const Normal &lhs, T a) -> Normal {
        T mean = a + lhs.get_mean();
        return Normal{mean, lhs.get_stddev()};
    }

    friend auto operator+(T a, const Normal &rhs) -> Normal { return rhs + a; }

    friend auto operator-(const Normal &lhs, T a) -> Normal {
        return lhs + (-a);
    }

    friend auto operator-(T a, const Normal &rhs) -> Normal {
        return a + (-rhs);
    }
};
