module;

#include <format>
#include <random>
#include <type_traits>
#include <vector>

export module diffusionx.random.gamma;

import diffusionx.error;
import diffusionx.random.utils;

using std::format;
using std::vector;

/**
 * @brief Generates a vector of gamma distributed random values
 * @tparam T The floating-point type for the generated values
 * @param n The number of values to generate
 * @param shape The shape parameter (α) of the gamma distribution (must be positive)
 * @param scale The scale parameter (β) of the gamma distribution (must be positive)
 * @return Result containing a vector of n gamma distributed values, or an Error
 * 
 * This function generates n random values from a gamma distribution with the
 * specified shape and scale parameters. The gamma distribution has probability
 * density function f(x) = (1/(Γ(α)β^α)) * x^(α-1) * e^(-x/β) for x > 0.
 * 
 * @note Uses parallel generation for improved performance
 * @note Each thread uses its own thread-local generator for thread safety
 */
export template<typename T = double>
    requires std::is_floating_point_v<T>
auto rand_gamma(size_t n, T shape, T scale) -> Result<vector<T> > {
    if (shape <= 0) {
        return Err(Error::InvalidArgument(
            format("The shape parameter `shape` must be positive, but got {}",
                   shape)));
    }

    if (scale <= 0) {
        return Err(Error::InvalidArgument(
            format("The scale parameter `scale` must be positive, but got {}",
                   scale)));
    }

    auto sampler = [shape, scale]() mutable {
        thread_local static std::mt19937 gen = generator();
        std::gamma_distribution<T> dist(shape, scale);
        return dist(gen);
    };
    return Ok(parallel_generate<T>(n, sampler));
}

/**
 * @brief Generates a single gamma distributed random value
 * @tparam T The floating-point type for the generated value
 * @param shape The shape parameter (α) of the gamma distribution (must be positive)
 * @param scale The scale parameter (β) of the gamma distribution (must be positive)
 * @return Result containing a gamma distributed value, or an Error
 * 
 * This function generates a single random value from a gamma distribution with
 * the specified shape and scale parameters. The gamma distribution has probability
 * density function f(x) = (1/(Γ(α)β^α)) * x^(α-1) * e^(-x/β) for x > 0.
 * 
 * @note Uses thread-local generator for thread safety
 */
export template<typename T = double>
    requires std::is_floating_point_v<T>
auto rand_gamma(T shape, T scale) -> Result<T> {
    if (shape <= 0) {
        return Err(Error::InvalidArgument(
            format("The shape parameter `shape` must be positive, but got {}",
                   shape)));
    }

    if (scale <= 0) {
        return Err(Error::InvalidArgument(
            format("The scale parameter `scale` must be positive, but got {}",
                   scale)));
    }

    thread_local static std::mt19937 gen = generator();
    std::gamma_distribution<T> dist(shape, scale);
    return Ok(dist(gen));
}

/**
 * @brief A class representing a gamma distribution
 * @tparam T The floating-point type for the distribution
 * 
 * This class encapsulates a gamma distribution with fixed shape and scale parameters.
 * It provides methods to sample from the distribution and access the parameters.
 * The gamma distribution is a versatile continuous probability distribution that
 * generalizes the exponential distribution and is commonly used in Bayesian statistics.
 */
export template<typename T = double>
    requires std::is_floating_point_v<T>
class Gamma {
    T m_shape; ///< The shape parameter (α) of the distribution
    T m_scale; ///< The scale parameter (β) of the distribution

public:
    /**
     * @brief Default constructor (parameters must be set before use)
     */
    Gamma() = default;

    /**
     * @brief Constructs a gamma distribution with specified shape and scale
     * @param shape The shape parameter (α) of the distribution (must be positive)
     * @param scale The scale parameter (β) of the distribution (must be positive)
     * @throws std::invalid_argument if either parameter is not positive
     */
    Gamma(T shape, T scale) : m_shape(shape), m_scale(scale) {
        if (m_shape <= 0) {
            throw std::invalid_argument(format(
                "The shape parameter `shape` must be positive, but got {}",
                m_shape));
        }

        if (m_scale <= 0) {
            throw std::invalid_argument(format(
                "The scale parameter `scale` must be positive, but got {}",
                m_scale));
        }
    }

    /**
     * @brief Gets the shape parameter of the distribution
     * @return The shape parameter (α)
     */
    [[nodiscard]] auto get_shape() const -> T { return m_shape; }

    /**
     * @brief Gets the scale parameter of the distribution
     * @return The scale parameter (β)
     */
    [[nodiscard]] auto get_scale() const -> T { return m_scale; }

    /**
     * @brief Generates multiple samples from the gamma distribution
     * @param n The number of samples to generate
     * @return Result containing a vector of n samples, or an Error
     * 
     * This method generates n independent samples from the gamma distribution
     * using the stored shape and scale parameters.
     */
    [[nodiscard]] auto sample(size_t n) const -> Result<vector<T> > {
        return rand_gamma(n, m_shape, m_scale);
    }
};
