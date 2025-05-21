#include "random/exponential.h"
#include "random/utils.h"

Result<vector<double> > randexp(size_t n, double rate) {
    if (rate <= 0) {
        return Err(Error::InvalidArgument(format(
            "The rate `rate` must be positive, but got {}",
            rate
        )));
    }
    auto sampler = [rate]() mutable {
        thread_local static std::mt19937 gen = generator();
        std::exponential_distribution dist(rate);
        return dist(gen);
    };
    return Ok(parallel_generate<double>(n, sampler));
}

double Exponential::get_rate() const {
    return m_rate;
}

Result<vector<double> > Exponential::sample(size_t n) const {
    return randexp(n, m_rate);
}
