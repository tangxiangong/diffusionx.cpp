module;

#include <cmath>
#include <cstddef>
#include <format>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>

export module diffusionx.random.stable;

import diffusionx.error;
import diffusionx.random.utils;
import diffusionx.random.uniform;
import diffusionx.random.exponential;

using std::vector;
using std::numbers::pi;

auto check_parameters(double alpha, double beta, double sigma) -> Result<bool> {
    if (alpha <= 0 || alpha > 2) {
        return Err(Error::InvalidArgument(std::format(
            "The stable index `alpha` must be in (0, 2], but got {}", alpha)));
    }

    if (beta < -1 || beta > 1) {
        return Err(Error::InvalidArgument(std::format(
            "The skewness parameter `beta` must be in [-1, 1], but got {}",
            beta)));
    }

    if (sigma <= 0) {
        return Err(Error::InvalidArgument(std::format(
            "The scale parameter `sigma` must be positive, but got {}",
            sigma)));
    }
    return Ok(true);
}

auto sample_standard(double alpha, double beta) -> double {
    double half_pi = pi / 2;
    double tmp = beta * tan(alpha * half_pi);
    double v = rand(-half_pi, half_pi).value();
    double w = randexp().value();
    double b = atan(tmp) / alpha;
    double s = pow(1 + tmp * tmp, 1 / (2 * alpha));
    double c1 = alpha * sin(v + b) / pow(cos(v), 1 / alpha);
    double c2 = pow(cos(v - alpha * (v + b)) / w, (1 - alpha) / alpha);
    return s * c1 * c2;
}

auto sample_standard(double beta) -> double {
    double half_pi = pi / 2;
    double v = rand(-half_pi, half_pi).value();
    double w = randexp().value();
    double c1 = (half_pi + beta * v) * tan(v);
    double tmp = (half_pi * w * cos(v)) / (half_pi + beta * v);
    double c2 = log(tmp) * beta;
    return 2 * (c1 - c2) / pi;
}

auto sample(double alpha, double beta, double sigma, double mu) -> double {
    double r = sample_standard(alpha, beta);
    return mu + sigma * r;
}

auto sample(double beta, double sigma, double mu) -> double {
    double r = sample_standard(beta);
    return sigma * r + mu + 2 * beta * sigma * sigma * log(sigma) / pi;
}

export auto rand_stable(double alpha, double beta = 0.0, double sigma = 1.0,
                        double mu = 0.0) -> Result<double> {
    if (auto res = check_parameters(alpha, beta, sigma); !res)
        return Err(res.error());
    if (alpha == 1) {
        return Ok(sample(beta, sigma, mu));
    } else {
        return Ok(sample(alpha, beta, sigma, mu));
    }
}

export auto rand_stable(size_t n, double alpha, double beta = 0.0,
                        double sigma = 1.0, double mu = 0.0)
    -> Result<vector<double>> {
    if (auto res = check_parameters(alpha, beta, sigma); !res)
        return Err(res.error());
    if (alpha == 1) {
        auto sampler = [beta, sigma, mu]() { return sample(beta, sigma, mu); };
        return Ok(parallel_generate<double>(n, sampler));
    } else {
        auto sampler = [alpha, beta, sigma, mu]() {
            return sample(alpha, beta, sigma, mu);
        };
        return Ok(parallel_generate<double>(n, sampler));
    }
}

export auto rand_skew_stable(double alpha) -> Result<double> {
    if (alpha <= 0 || alpha >= 1) {
        return Err(Error::InvalidArgument(std::format(
            "The stable index `alpha` must be in (0, 1), but got {}", alpha)));
    }
    return Ok(sample(alpha, 1.0, 1.0, 0.0));
}

export auto rand_skew_stable(size_t n, double alpha) -> Result<vector<double>> {
    if (alpha <= 0 || alpha >= 1) {
        return Err(Error::InvalidArgument(std::format(
            "The stable index `alpha` must be in (0, 1), but got {}", alpha)));
    }
    auto sampler = [alpha]() { return sample(alpha, 1.0, 1.0, 0.0); };
    return Ok(parallel_generate<double>(n, sampler));
}

export class Stable {
    double m_alpha;
    double m_beta;
    double m_sigma;
    double m_mu;

   public:
    Stable() = default;

    explicit Stable(double alpha, double beta = 0.0, double sigma = 1.0,
                    double mu = 0.0);

    [[nodiscard]] auto get_alpha() const -> double;

    [[nodiscard]] auto get_beta() const -> double;

    [[nodiscard]] auto get_sigma() const -> double;

    [[nodiscard]] auto get_mu() const -> double;

    [[nodiscard]] auto sample(size_t n) const -> Result<vector<double>>;
};

Stable::Stable(double alpha, double beta, double sigma, double mu) {
    if (auto res = check_parameters(alpha, beta, sigma); !res) {
        throw std::invalid_argument(res.error().message);
    }
    m_alpha = alpha;
    m_beta = beta;
    m_sigma = sigma;
    m_mu = mu;
}

auto Stable::get_alpha() const -> double { return m_alpha; }

auto Stable::get_beta() const -> double { return m_beta; }

auto Stable::get_sigma() const -> double { return m_sigma; }

auto Stable::get_mu() const -> double { return m_mu; }

auto Stable::sample(size_t n) const -> Result<vector<double>> {
    return rand_stable(n, m_alpha, m_beta, m_sigma, m_mu);
}
