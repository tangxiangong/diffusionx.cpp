#ifndef NORMAL_H
#define NORMAL_H
#include <vector>
#include <format>
#include "../error.h"

using std::format;

namespace normal {
    Result<std::vector<double> > rands(size_t n = 1, double mean = 0.0, double stddev = 1.0);

    class Normal {
    private:
        double m_mean = 0.0;
        double m_stddev = 1.0;

    public:
        Normal() = default;

        Normal(const double mean, const double stddev) : m_mean(mean), m_stddev(stddev) {
            if (m_stddev <= 0) {
                throw std::invalid_argument(format(
                    "The standard deviation `stddev` must be positive, but got {}",
                    m_stddev
                ));
            }
        }

        [[nodiscard]] double get_mean() const;

        [[nodiscard]] double get_stddev() const;

        [[nodiscard]] Result<std::vector<double> > sample(size_t n) const;

        Normal operator+(const Normal &rhs) const;

        Normal operator-(const Normal &rhs) const;

        Normal operator-() const;

        friend Result<Normal> operator*(double a, const Normal &rhs);

        friend Result<Normal> operator*(const Normal &lhs, double a);

        friend Normal operator+(double a, const Normal &rhs);

        friend Normal operator-(double a, const Normal &rhs);

        friend Normal operator+(const Normal &lhs, double a);

        friend Normal operator-(const Normal &lhs, double a);
    };
}
#endif //NORMAL_H
