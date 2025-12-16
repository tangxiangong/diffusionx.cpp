module;

#include <format>
#include <random>
#include <vector>

export module diffusionx.random.normal;

import diffusionx.error;
import diffusionx.random.utils;

using std::format;
using std::vector;

/**
 * @brief Generates a vector of normally distributed random values
 * @tparam T The floating-point type for the generated values
 * @param n The number of values to generate
 * @param mean The mean (μ) of the normal distribution
 * @param stddev The standard deviation (σ) of the normal distribution (must be
 * positive)
 * @return Result containing a vector of n normally distributed values, or an
 * Error
 *
 * This function generates n random values from a normal (Gaussian) distribution
 * with the specified mean and standard deviation. The normal distribution has
 * probability density function f(x) = (1/(σ√(2π))) * e^(-(x-μ)²/(2σ²)).
 *
 * @note Uses parallel generation for improved performance
 * @note Each thread uses its own thread-local generator for thread safety
 */
export template <Float T = double>
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

/**
 * @brief Generates a single normally distributed random value
 * @tparam T The floating-point type for the generated value
 * @param mean The mean (μ) of the normal distribution
 * @param stddev The standard deviation (σ) of the normal distribution (must be
 * positive)
 * @return Result containing a normally distributed value, or an Error
 *
 * This function generates a single random value from a normal (Gaussian)
 * distribution with the specified mean and standard deviation. The normal
 * distribution has probability density function f(x) = (1/(σ√(2π))) *
 * e^(-(x-μ)²/(2σ²)).
 *
 * @note Uses thread-local generator for thread safety
 */
export template <Float T = double> auto randn(T mean, T stddev) -> Result<T> {
    if (stddev <= 0) {
        return Err(Error::InvalidArgument(format(
            "The standard deviation `stddev` must be positive, but got {}",
            stddev)));
    }
    thread_local static std::mt19937 gen = generator();
    std::normal_distribution<T> dist(mean, stddev);
    return Ok(dist(gen));
}

export template <Float T = double> auto randn() -> T {
    thread_local static std::mt19937 gen = generator();
    std::normal_distribution<T> dist(0, 1);
    return dist(gen);
}

/**
 * @brief A class representing a normal (Gaussian) distribution
 * @tparam T The floating-point type for the distribution
 *
 * This class encapsulates a normal distribution with fixed mean and standard
 * deviation parameters. It provides methods to sample from the distribution,
 * access parameters, and perform arithmetic operations that preserve the normal
 * distribution property. The normal distribution is fundamental in statistics
 * and probability theory.
 */
export template <Float T> class Normal {
    T m_mean = 0.0;   ///< The mean (μ) of the distribution
    T m_stddev = 1.0; ///< The standard deviation (σ) of the distribution

  public:
    /**
     * @brief Default constructor creating a standard normal distribution (μ=0,
     * σ=1)
     */
    Normal() = default;

    /**
     * @brief Constructs a normal distribution with specified mean and standard
     * deviation
     * @param mean The mean (μ) of the distribution
     * @param stddev The standard deviation (σ) of the distribution (must be
     * positive)
     * @throws std::invalid_argument if stddev is not positive
     */
    Normal(T mean, T stddev) : m_mean(mean), m_stddev(stddev) {
        if (m_stddev <= 0) {
            throw std::invalid_argument(format(
                "The standard deviation `stddev` must be positive, but got {}",
                m_stddev));
        }
    }

    /**
     * @brief Gets the mean parameter of the distribution
     * @return The mean (μ)
     */
    [[nodiscard]] auto get_mean() const -> T { return m_mean; }

    /**
     * @brief Gets the standard deviation parameter of the distribution
     * @return The standard deviation (σ)
     */
    [[nodiscard]] auto get_stddev() const -> T { return m_stddev; }

    /**
     * @brief Generates multiple samples from the normal distribution
     * @param n The number of samples to generate
     * @return Result containing a vector of n samples, or an Error
     *
     * This method generates n independent samples from the normal distribution
     * using the stored mean and standard deviation parameters.
     */
    [[nodiscard]] auto sample(size_t n) const -> Result<vector<T>> {
        return randn(n, m_mean, m_stddev);
    }

    /**
     * @brief Generates a sample from the normal distribution
     * @return Result containing a vector of n samples, or an Error
     *
     */
    [[nodiscard]] auto sample() const -> Result<T> {
        return randn(m_mean, m_stddev);
    }
};
