#ifndef STABLE_H
#define STABLE_H
#include <vector>
#include <format>
#include "../error.hpp"


using std::vector;
using std::format;

Result<double> rand_stable(double alpha, double beta = 0.0, double sigma = 1.0, double mu = 0.0);

Result<vector<double> > rand_stable(size_t n, double alpha, double beta = 0.0,
                                    double sigma = 1.0, double mu = 0.0);

Result<double> rand_skew_stable(double alpha);

Result<vector<double> > rand_skew_stable(size_t n, double alpha);

class Stable {
    double m_alpha;
    double m_beta;
    double m_sigma;
    double m_mu;

public:
    Stable() = default;

    explicit Stable(double alpha, double beta = 0.0, double sigma = 1.0, double mu = 0.0);

    [[nodiscard]] double get_alpha() const;

    [[nodiscard]] double get_beta() const;

    [[nodiscard]] double get_sigma() const;

    [[nodiscard]] double get_mu() const;

    [[nodiscard]] Result<vector<double>> sample(size_t n) const;
};
#endif //STABLE_H
