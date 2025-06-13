module;

#include <cmath>
#include <format>
#include <numbers>
#include <stdexcept>
#include <vector>

export module diffusionx.random.stable;

import diffusionx.error;
import diffusionx.random.utils;
import diffusionx.random.uniform;
import diffusionx.random.exponential;

using std::vector;
using std::numbers::pi;

/**
 * @brief Validates the parameters of a stable distribution
 * @param alpha The stability parameter (must be in (0, 2])
 * @param beta The skewness parameter (must be in [-1, 1])
 * @param sigma The scale parameter (must be positive)
 * @return Result containing true if parameters are valid, or an Error
 * 
 * This function checks that the parameters satisfy the constraints for a
 * valid stable distribution.
 */
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

/**
 * @brief Generates a sample from the standard stable distribution (α ≠ 1)
 * @param alpha The stability parameter (must be in (0, 2] and ≠ 1)
 * @param beta The skewness parameter (must be in [-1, 1])
 * @return A sample from the standard stable distribution
 * 
 * This function implements the Chambers-Mallows-Stuck algorithm for generating
 * samples from a stable distribution when α ≠ 1.
 */
auto sample_standard(double alpha, double beta) -> double {
    double half_pi = pi / 2;
    double tmp = beta * tan(alpha * half_pi);
    double v = rand(-half_pi, half_pi).value();
    double w = randexp().value();
    double b = atan(tmp) / alpha;
    double s = pow(1 + (tmp * tmp), 1 / (2 * alpha));
    double c1 = alpha * sin(v + b) / pow(cos(v), 1 / alpha);
    double c2 = pow(cos(v - (alpha * (v + b))) / w, (1 - alpha) / alpha);
    return s * c1 * c2;
}

/**
 * @brief Generates a sample from the standard stable distribution (α = 1)
 * @param beta The skewness parameter (must be in [-1, 1])
 * @return A sample from the standard stable distribution with α = 1
 * 
 * This function implements the special case algorithm for generating samples
 * from a stable distribution when α = 1 (Cauchy-like distribution).
 */
auto sample_standard(double beta) -> double {
    double half_pi = pi / 2;
    double v = rand(-half_pi, half_pi).value();
    double w = randexp().value();
    double c1 = (half_pi + beta * v) * tan(v);
    double tmp = half_pi * w * cos(v) / (half_pi + beta * v);
    double c2 = log(tmp) * beta;
    return 2 * (c1 - c2) / pi;
}

/**
 * @brief Generates a sample from a stable distribution (α ≠ 1)
 * @param alpha The stability parameter
 * @param beta The skewness parameter
 * @param sigma The scale parameter
 * @param mu The location parameter
 * @return A sample from the stable distribution
 */
auto sample(double alpha, double beta, double sigma, double mu) -> double {
    double r = sample_standard(alpha, beta);
    return mu + (sigma * r);
}

/**
 * @brief Generates a sample from a stable distribution (α = 1)
 * @param beta The skewness parameter
 * @param sigma The scale parameter
 * @param mu The location parameter
 * @return A sample from the stable distribution with α = 1
 */
auto sample(double beta, double sigma, double mu) -> double {
    double r = sample_standard(beta);
    return (sigma * r) + mu + (2 * beta * sigma * sigma * log(sigma) / pi);
}

/**
 * @brief Generates a single stable distributed random value
 * @param alpha The stability parameter (must be in (0, 2])
 * @param beta The skewness parameter (must be in [-1, 1])
 * @param sigma The scale parameter (must be positive)
 * @param mu The location parameter
 * @return Result containing a stable distributed value, or an Error
 * 
 * This function generates a single random value from a stable distribution
 * with the specified parameters. Stable distributions are a family of
 * probability distributions that generalize the normal distribution and
 * are characterized by their stability under addition.
 */
export auto rand_stable(double alpha, double beta = 0.0, double sigma = 1.0,
                        double mu = 0.0) -> Result<double> {
    if (auto res = check_parameters(alpha, beta, sigma); !res) {
        return Err(res.error());
    }
    if (alpha == 1) {
        return Ok(sample(beta, sigma, mu));
    }
    return Ok(sample(alpha, beta, sigma, mu));
}

/**
 * @brief Generates a vector of stable distributed random values
 * @param n The number of values to generate
 * @param alpha The stability parameter (must be in (0, 2])
 * @param beta The skewness parameter (must be in [-1, 1])
 * @param sigma The scale parameter (must be positive)
 * @param mu The location parameter
 * @return Result containing a vector of n stable distributed values, or an Error
 * 
 * This function generates n random values from a stable distribution with the
 * specified parameters. Uses parallel generation for improved performance.
 */
export auto rand_stable(size_t n, double alpha, double beta = 0.0,
                        double sigma = 1.0, double mu = 0.0)
    -> Result<vector<double> > {
    if (auto res = check_parameters(alpha, beta, sigma); !res) {
        return Err(res.error());
    }
    if (alpha == 1) {
        auto sampler = [beta, sigma, mu]() { return sample(beta, sigma, mu); };
        return Ok(parallel_generate<double>(n, sampler));
    }
    auto sampler = [alpha, beta, sigma, mu]() {
        return sample(alpha, beta, sigma, mu);
    };
    return Ok(parallel_generate<double>(n, sampler));
}

/**
 * @brief Generates a single maximally skewed stable distributed random value
 * @param alpha The stability parameter (must be in (0, 1))
 * @return Result containing a maximally skewed stable distributed value, or an Error
 * 
 * This function generates a single random value from a maximally skewed stable
 * distribution (β = 1) with unit scale and zero location. This is a special
 * case often used in modeling heavy-tailed phenomena.
 */
export auto rand_skew_stable(double alpha) -> Result<double> {
    if (alpha <= 0 || alpha >= 1) {
        return Err(Error::InvalidArgument(std::format(
            "The stable index `alpha` must be in (0, 1), but got {}", alpha)));
    }
    return Ok(sample(alpha, 1.0, 1.0, 0.0));
}

/**
 * @brief Generates a vector of maximally skewed stable distributed random values
 * @param n The number of values to generate
 * @param alpha The stability parameter (must be in (0, 1))
 * @return Result containing a vector of n maximally skewed stable distributed values, or an Error
 * 
 * This function generates n random values from a maximally skewed stable
 * distribution (β = 1) with unit scale and zero location.
 */
export auto rand_skew_stable(size_t n, double alpha) -> Result<vector<double> > {
    if (alpha <= 0 || alpha >= 1) {
        return Err(Error::InvalidArgument(std::format(
            "The stable index `alpha` must be in (0, 1), but got {}", alpha)));
    }
    auto sampler = [alpha]() { return sample(alpha, 1.0, 1.0, 0.0); };
    return Ok(parallel_generate<double>(n, sampler));
}

/**
 * @brief A class representing a stable distribution
 * 
 * This class encapsulates a stable distribution with fixed parameters.
 * It provides methods to sample from the distribution and access the parameters.
 * Stable distributions are a rich family of probability distributions that
 * include the normal, Cauchy, and Lévy distributions as special cases.
 */
export class Stable {
    double m_alpha; ///< The stability parameter (α ∈ (0, 2])
    double m_beta; ///< The skewness parameter (β ∈ [-1, 1])
    double m_sigma; ///< The scale parameter (σ > 0)
    double m_mu; ///< The location parameter (μ ∈ ℝ)

public:
    /**
     * @brief Default constructor (parameters must be set before use)
     */
    Stable() = default;

    /**
     * @brief Constructs a stable distribution with specified parameters
     * @param alpha The stability parameter (must be in (0, 2])
     * @param beta The skewness parameter (must be in [-1, 1])
     * @param sigma The scale parameter (must be positive)
     * @param mu The location parameter
     * @throws std::invalid_argument if parameters are invalid
     */
    explicit Stable(double alpha, double beta = 0.0, double sigma = 1.0,
                    double mu = 0.0);

    /**
     * @brief Gets the stability parameter of the distribution
     * @return The stability parameter (α)
     */
    [[nodiscard]] auto get_alpha() const -> double;

    /**
     * @brief Gets the skewness parameter of the distribution
     * @return The skewness parameter (β)
     */
    [[nodiscard]] auto get_beta() const -> double;

    /**
     * @brief Gets the scale parameter of the distribution
     * @return The scale parameter (σ)
     */
    [[nodiscard]] auto get_sigma() const -> double;

    /**
     * @brief Gets the location parameter of the distribution
     * @return The location parameter (μ)
     */
    [[nodiscard]] auto get_mu() const -> double;

    /**
     * @brief Generates multiple samples from the stable distribution
     * @param n The number of samples to generate
     * @return Result containing a vector of n samples, or an Error
     * 
     * This method generates n independent samples from the stable distribution
     * using the stored parameters.
     */
    [[nodiscard]] auto sample(size_t n) const -> Result<vector<double> >;
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

auto Stable::sample(size_t n) const -> Result<vector<double> > {
    return rand_stable(n, m_alpha, m_beta, m_sigma, m_mu);
}
