#include "random/normal.h"
#include "random/utils.h"
#include <random>
#include <format>


using std::vector;
using std::format;

namespace normal {
    Result<vector<double>> rands(size_t n, double mean, double stddev) {
        if (stddev <= 0) {
            return Err(Error::InvalidArgument(format(
                "The standard deviation `stddev` must be positive, but got {}",
                stddev
            )));
        }
        auto sampler = [mean, stddev]() mutable {
            thread_local std::mt19937 gen{std::random_device{}()};
            thread_local std::normal_distribution<double> dist{mean, stddev};
            return dist(gen);
        };
        return Ok(parallel_sample<double>(n, sampler));
    }

    double Normal::get_mean() const {
        return m_mean;
    }

    double Normal::get_stddev() const {
        return m_stddev;
    }

    Result<std::vector<double>> Normal::sample(const size_t n) const {
        return rands(n, m_mean, m_stddev);
    }

}