module;

#include <format>
#include <random>
#include <vector>

export module diffusionx.random.poisson;

import diffusionx.error;
import diffusionx.random.utils;

using std::format;
using std::vector;

/**
 * @brief Generates a vector of Poisson distributed random values
 * @tparam T The unsigned integer type for the generated values
 * @param n The number of values to generate
 * @param rate The rate parameter (λ) of the Poisson distribution (must be positive)
 * @return Result containing a vector of n Poisson distributed values, or an Error
 * 
 * This function generates n random values from a Poisson distribution with the
 * specified rate parameter. The Poisson distribution has probability mass
 * function P(X = k) = (λ^k * e^(-λ)) / k! for k = 0, 1, 2, ...
 * 
 * @note Uses parallel generation for improved performance
 * @note Each thread uses its own thread-local generator for thread safety
 */
export template<typename T = unsigned int>
    requires std::is_unsigned_v<T>
auto rand_poisson(size_t n, double rate = 1.0) -> Result<vector<T> > {
    if (rate <= 0) {
        return Err(Error::InvalidArgument(
            format("The rate `rate` must be positive, but got {}", rate)));
    }
    auto sampler = [rate]() mutable {
        thread_local static std::mt19937 gen = generator();
        std::poisson_distribution<T> dist(rate);
        return dist(gen);
    };
    return Ok(parallel_generate<T>(n, sampler));
}

/**
 * @brief Generates a single Poisson distributed random value
 * @tparam T The unsigned integer type for the generated value
 * @param rate The rate parameter (λ) of the Poisson distribution (must be positive)
 * @return Result containing a Poisson distributed value, or an Error
 * 
 * This function generates a single random value from a Poisson distribution with
 * the specified rate parameter. The Poisson distribution has probability mass
 * function P(X = k) = (λ^k * e^(-λ)) / k! for k = 0, 1, 2, ...
 * 
 * @note Uses thread-local generator for thread safety
 */
export template<typename T = unsigned int>
    requires std::is_unsigned_v<T>
auto rand_poisson(double rate = 1.0) -> Result<T> {
    if (rate <= 0) {
        return Err(Error::InvalidArgument(
            format("The rate `rate` must be positive, but got {}", rate)));
    }
    thread_local static std::mt19937 gen = generator();
    std::poisson_distribution<T> dist(rate);
    return Ok(dist(gen));
}

/**
 * @brief A class representing a Poisson distribution
 * 
 * This class encapsulates a Poisson distribution with a fixed rate parameter.
 * It provides methods to sample from the distribution and access the parameters.
 * The Poisson distribution is commonly used to model the number of events
 * occurring in a fixed interval of time or space, given a constant average rate.
 */
export class Poisson {
    double m_rate = 1.0; ///< The rate parameter (λ) of the distribution

public:
    /**
     * @brief Default constructor creating a Poisson distribution with rate = 1.0
     */
    Poisson() = default;

    /**
     * @brief Constructs a Poisson distribution with the specified rate
     * @param rate The rate parameter (λ) of the distribution (must be positive)
     * @throws std::invalid_argument if rate is not positive
     */
    explicit Poisson(double rate) : m_rate(rate) {
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
    [[nodiscard]] auto get_rate() const -> double { return m_rate; }

    /**
     * @brief Generates multiple samples from the Poisson distribution
     * @tparam T The unsigned integer type for the samples (default: unsigned int)
     * @param n The number of samples to generate
     * @return Result containing a vector of n samples, or an Error
     * 
     * This method generates n independent samples from the Poisson distribution
     * using the stored rate parameter.
     */
    template<typename T = unsigned int>
        requires std::is_unsigned_v<T>
    [[nodiscard]] auto sample(size_t n) const -> Result<vector<T> > {
        return rand_poisson<T>(n, m_rate);
    }
};
