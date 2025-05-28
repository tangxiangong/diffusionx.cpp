module;

#include <format>
#include <random>
#include <type_traits>
#include <vector>

export module diffusionx.random.exponential;

import diffusionx.error;
import diffusionx.random.utils;

using std::format;
using std::vector;

/**
 * @brief Generates a vector of exponentially distributed random values
 * @tparam T The floating-point type for the generated values
 * @param n The number of values to generate
 * @param rate The rate parameter (λ) of the exponential distribution (must be positive)
 * @return Result containing a vector of n exponentially distributed values, or an Error
 * 
 * This function generates n random values from an exponential distribution with
 * the specified rate parameter. The exponential distribution has probability
 * density function f(x) = λe^(-λx) for x ≥ 0.
 * 
 * @note Uses parallel generation for improved performance
 * @note Each thread uses its own thread-local generator for thread safety
 */
export template<typename T = double>
    requires std::is_floating_point_v<T>
auto randexp(size_t n, T rate = 1.0) -> Result<vector<T> > {
    if (rate <= 0) {
        return Err(Error::InvalidArgument(
            format("The rate `rate` must be positive, but got {}", rate)));
    }
    auto sampler = [rate]() mutable {
        thread_local static std::mt19937 gen = generator();
        std::exponential_distribution<T> dist(rate);
        return dist(gen);
    };
    return Ok(parallel_generate<T>(n, sampler));
}

/**
 * @brief Generates a single exponentially distributed random value
 * @tparam T The floating-point type for the generated value
 * @param rate The rate parameter (λ) of the exponential distribution (must be positive)
 * @return Result containing an exponentially distributed value, or an Error
 * 
 * This function generates a single random value from an exponential distribution
 * with the specified rate parameter. The exponential distribution has probability
 * density function f(x) = λe^(-λx) for x ≥ 0.
 * 
 * @note Uses thread-local generator for thread safety
 */
export template<typename T = double>
    requires std::is_floating_point_v<T>
auto randexp(T rate = 1.0) -> Result<T> {
    if (rate <= 0) {
        return Err(Error::InvalidArgument(
            format("The rate `rate` must be positive, but got {}", rate)));
    }
    thread_local static std::mt19937 gen = generator();
    std::exponential_distribution<T> dist(rate);
    return Ok(dist(gen));
}

/**
 * @brief A class representing an exponential distribution
 * @tparam T The floating-point type for the distribution
 * 
 * This class encapsulates an exponential distribution with a fixed rate parameter.
 * It provides methods to sample from the distribution and access the parameters.
 * The exponential distribution is commonly used to model waiting times between
 * events in a Poisson process.
 */
export template<typename T = double>
    requires std::is_floating_point_v<T>
class Exponential {
    T m_rate = 1.0; ///< The rate parameter (λ) of the distribution

public:
    /**
     * @brief Default constructor creating an exponential distribution with rate = 1.0
     */
    Exponential() = default;

    /**
     * @brief Constructs an exponential distribution with the specified rate
     * @param rate The rate parameter (λ) of the distribution (must be positive)
     * @throws std::invalid_argument if rate is not positive
     */
    explicit Exponential(T rate) : m_rate(rate) {
        if (m_rate <= 0) {
            throw std::invalid_argument(
                format("The rate parameter `rate` must be positive, but got {}",
                       m_rate));
        }
    }

    /**
     * @brief Gets the rate parameter of the distribution
     * @return The rate parameter (λ)
     */
    [[nodiscard]] auto get_rate() const -> T { return m_rate; }

    /**
     * @brief Generates multiple samples from the exponential distribution
     * @param n The number of samples to generate
     * @return Result containing a vector of n samples, or an Error
     * 
     * This method generates n independent samples from the exponential distribution
     * using the stored rate parameter.
     */
    [[nodiscard]] auto sample(size_t n) const -> Result<vector<T> > {
        return randexp(n, m_rate);
    }
};
