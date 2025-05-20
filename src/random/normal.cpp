#include "random/normal.h"
#include "random/utils.h"
#include <cmath>

using std::vector;
using std::format;

namespace normal {
    Result<vector<double> > rands(size_t n, double mean, double stddev) {
        if (stddev <= 0) {
            return Err(Error::InvalidArgument(format(
                "The standard deviation `stddev` must be positive, but got {}",
                stddev
            )));
        }
        auto sampler = [mean, stddev]() mutable {
            thread_local static std::mt19937 gen = generator();
            std::normal_distribution<double> dist(mean, stddev);
            return dist(gen);
        };
        return Ok(parallel_generate<double>(n, sampler));
    }

    double Normal::get_mean() const {
        return m_mean;
    }

    double Normal::get_stddev() const {
        return m_stddev;
    }

    Result<std::vector<double> > Normal::sample(const size_t n) const {
        return rands(n, m_mean, m_stddev);
    }

    Normal Normal::operator+(const Normal &rhs) const {
        double stddev = std::sqrt(m_stddev * m_stddev + rhs.get_stddev() * rhs.get_stddev());
        double mean = m_mean + rhs.get_mean();
        return Normal{mean, stddev};
    }

    Normal Normal::operator-() const {
        return Normal{-m_mean, m_stddev};
    }

    Normal Normal::operator-(const Normal &rhs) const {
        return *this + (-rhs);
    }

    Result<Normal> operator*(double a, const Normal &rhs) {
        if (a == 0.0) {
            return Err(Error::InvalidArgument("The scale number should not be zero."));
        }
        double mean = a * rhs.get_mean();
        double stddev = std::abs(a) * rhs.get_stddev();
        return Ok(Normal{mean, stddev});
    }

    Result<Normal> operator*(const Normal &lhs, double a) {
        return a * lhs;
    }

    Normal operator+(const Normal &lhs, double a) {
        double mean = a + lhs.get_mean();
        return Normal{mean, lhs.get_stddev()};
    }

    Normal operator+(double a, const Normal &rhs) {
        return rhs + a;
    }

    Normal operator-(const Normal &lhs, double a) {
        return lhs + (-a);
    }

    Normal operator-(double a, const Normal &rhs) {
        return a + (-rhs);
    }
}
