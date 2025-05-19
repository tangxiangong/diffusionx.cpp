#ifndef NORMAL_H
#define NORMAL_H
#include <vector>
#include "../error.h"

namespace normal {
    Result<std::vector<double>> rands(size_t n = 1, double mu = 0.0, double sigma = 1.0);

    class Normal {
    private:
        double m_mean = 0.0;
        double m_stddev = 1.0;
    public:
        Normal() = default;
        Normal(const double mean, const double stddev) : m_mean(mean), m_stddev(stddev) {}
        [[nodiscard]] double get_mean() const;
        [[nodiscard]] double get_stddev() const;
        [[nodiscard]] Result<std::vector<double>> sample(size_t n) const;
    };
}
#endif //NORMAL_H
