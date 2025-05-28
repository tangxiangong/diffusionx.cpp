module;

#include <format>
#include <random>
#include <type_traits>
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
 * @param stddev The standard deviation (σ) of the normal distribution (must be positive)
 * @return Result containing a vector of n normally distributed values, or an Error
 * 
 * This function generates n random values from a normal (Gaussian) distribution
 * with the specified mean and standard deviation. The normal distribution has
 * probability density function f(x) = (1/(σ√(2π))) * e^(-(x-μ)²/(2σ²)).
 * 
 * @note Uses parallel generation for improved performance
 * @note Each thread uses its own thread-local generator for thread safety
 */
export template<typename T = double>
    requires std::is_floating_point_v<T>
auto randn(size_t n, T mean = 0, T stddev = 1) -> Result<vector<T> > {
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
 * @param stddev The standard deviation (σ) of the normal distribution (must be positive)
 * @return Result containing a normally distributed value, or an Error
 * 
 * This function generates a single random value from a normal (Gaussian) distribution
 * with the specified mean and standard deviation. The normal distribution has
 * probability density function f(x) = (1/(σ√(2π))) * e^(-(x-μ)²/(2σ²)).
 * 
 * @note Uses thread-local generator for thread safety
 */
export template<typename T = double>
    requires std::is_floating_point_v<T>
auto randn(T mean = 0, T stddev = 1) -> Result<T> {
    if (stddev <= 0) {
        return Err(Error::InvalidArgument(format(
            "The standard deviation `stddev` must be positive, but got {}",
            stddev)));
    }
    thread_local static std::mt19937 gen = generator();
    std::normal_distribution<T> dist(mean, stddev);
    return Ok(dist(gen));
}

/**
 * @brief A class representing a normal (Gaussian) distribution
 * @tparam T The floating-point type for the distribution
 * 
 * This class encapsulates a normal distribution with fixed mean and standard deviation
 * parameters. It provides methods to sample from the distribution, access parameters,
 * and perform arithmetic operations that preserve the normal distribution property.
 * The normal distribution is fundamental in statistics and probability theory.
 */
export template<typename T>
    requires std::is_floating_point_v<T>
class Normal {
    T m_mean = 0.0; ///< The mean (μ) of the distribution
    T m_stddev = 1.0; ///< The standard deviation (σ) of the distribution

public:
    /**
     * @brief Default constructor creating a standard normal distribution (μ=0, σ=1)
     */
    Normal() = default;

    /**
     * @brief Constructs a normal distribution with specified mean and standard deviation
     * @param mean The mean (μ) of the distribution
     * @param stddev The standard deviation (σ) of the distribution (must be positive)
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
    [[nodiscard]] auto sample(size_t n) const -> Result<vector<T> > {
        return randn(n, m_mean, m_stddev);
    }

    /**
     * @brief Adds two independent normal distributions
     * @param rhs The other normal distribution to add
     * @return A new normal distribution representing the sum
     * 
     * If X ~ N(μ₁, σ₁²) and Y ~ N(μ₂, σ₂²) are independent, then
     * X + Y ~ N(μ₁ + μ₂, σ₁² + σ₂²).
     */
    auto operator+(const Normal &rhs) const -> Normal {
        double stddev = std::sqrt((m_stddev * m_stddev) +
                                  (rhs.get_stddev() * rhs.get_stddev()));
        double mean = m_mean + rhs.get_mean();
        return Normal{mean, stddev};
    }

    /**
     * @brief Negates a normal distribution
     * @return A new normal distribution representing the negation
     * 
     * If X ~ N(μ, σ²), then -X ~ N(-μ, σ²).
     */
    auto operator-() const -> Normal { return Normal{-m_mean, m_stddev}; }

    /**
     * @brief Subtracts two independent normal distributions
     * @param rhs The normal distribution to subtract
     * @return A new normal distribution representing the difference
     * 
     * If X ~ N(μ₁, σ₁²) and Y ~ N(μ₂, σ₂²) are independent, then
     * X - Y ~ N(μ₁ - μ₂, σ₁² + σ₂²).
     */
    auto operator-(const Normal &rhs) const -> Normal { return *this + (-rhs); }

    /**
     * @brief Multiplies a normal distribution by a scalar
     * @param a The scalar multiplier
     * @param rhs The normal distribution to scale
     * @return A new normal distribution representing the scaled distribution
     * 
     * If X ~ N(μ, σ²), then aX ~ N(aμ, a²σ²).
     * 
     * @note Returns an error if the scalar is zero
     */
    friend auto operator*(T a, const Normal &rhs) -> Normal {
        if (a == 0.0) {
            return Err(
                Error::InvalidArgument("The scale number should not be zero."));
        }
        T mean = a * rhs.get_mean();
        T stddev = std::abs(a) * rhs.get_stddev();
        return Ok(Normal{mean, stddev});
    }

    /**
     * @brief Multiplies a normal distribution by a scalar (commutative)
     * @param lhs The normal distribution to scale
     * @param a The scalar multiplier
     * @return A new normal distribution representing the scaled distribution
     */
    friend auto operator*(const Normal &lhs, T a) -> Normal { return a * lhs; }

    /**
     * @brief Adds a constant to a normal distribution
     * @param lhs The normal distribution
     * @param a The constant to add
     * @return A new normal distribution with shifted mean
     * 
     * If X ~ N(μ, σ²), then X + a ~ N(μ + a, σ²).
     */
    friend auto operator+(const Normal &lhs, T a) -> Normal {
        T mean = a + lhs.get_mean();
        return Normal{mean, lhs.get_stddev()};
    }

    /**
     * @brief Adds a constant to a normal distribution (commutative)
     * @param a The constant to add
     * @param rhs The normal distribution
     * @return A new normal distribution with shifted mean
     */
    friend auto operator+(T a, const Normal &rhs) -> Normal { return rhs + a; }

    /**
     * @brief Subtracts a constant from a normal distribution
     * @param lhs The normal distribution
     * @param a The constant to subtract
     * @return A new normal distribution with shifted mean
     */
    friend auto operator-(const Normal &lhs, T a) -> Normal {
        return lhs + (-a);
    }

    /**
     * @brief Subtracts a normal distribution from a constant
     * @param a The constant
     * @param rhs The normal distribution to subtract
     * @return A new normal distribution representing the difference
     */
    friend auto operator-(T a, const Normal &rhs) -> Normal {
        return a + (-rhs);
    }
};
