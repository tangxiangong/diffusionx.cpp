#ifndef EXPONENTIAL_H
#define EXPONENTIAL_H

#include <vector>
#include <format>
#include "../error.h"

using std::vector;
using std::format;

Result<vector<double> > randexp(size_t n = 1, double rate = 1.0);

class Exponential {
    double m_rate = 1.0;

public:
    Exponential() = default;

    explicit Exponential(double rate) : m_rate(rate) {
        if (m_rate <= 0) {
            throw std::invalid_argument(format(
                "The rate parameter `rate` must be positive, but got {}",
                m_rate
            ));
        }
    }

    [[nodiscard]] double get_rate() const;

    [[nodiscard]] Result<vector<double> > sample(size_t n) const;
};

#endif //EXPONENTIAL_H
