/**
 * @file cauchy.cppm
 * @brief Cauchy process implementation
 *
 * This module provides the implementation of the Cauchy process.
 */

module;

#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.cauchy;

import diffusionx.error;
import diffusionx.random.stable;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Cauchy process implementation
 *
 * This class implements the Cauchy process, which is a special case of
 * the Lévy process with α = 1 and β = 0 (symmetric Cauchy distribution).
 *
 * Mathematical definition:
 * X(t) = X(0) + C(t)
 *
 * where C(t) is a Cauchy process with independent increments
 * C(t + s) - C(t) ~ Cauchy(0, σ√s) for s > 0
 */
export class Cauchy : public ContinuousProcess {
  double m_sigma = 1.0;          ///< Scale parameter σ > 0
  double m_start_position = 0.0; ///< Initial position

public:
  /**
   * @brief Default constructor creating standard Cauchy process
   */
  Cauchy() = default;

  /**
   * @brief Constructs Cauchy process with specified parameters
   * @param sigma Scale parameter σ (must be positive)
   * @param start_position Initial position
   * @throws std::invalid_argument if sigma is not positive
   */
  Cauchy(double sigma, double start_position = 0.0)
      : m_sigma(sigma), m_start_position(start_position) {
    if (m_sigma <= 0.0) {
      throw std::invalid_argument("Scale parameter sigma must be positive");
    }
  }

  /**
   * @brief Gets the scale parameter
   * @return The scale parameter σ
   */
  [[nodiscard]] auto get_sigma() const -> double { return m_sigma; }

  /**
   * @brief Gets the initial position
   * @return The initial position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Simulates a trajectory of the Cauchy process
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses independent Cauchy increments:
   * X(t + dt) = X(t) + Cauchy(0, σ√dt)
   * which is equivalent to Stable(1, 0, σ√dt, 0)
   */
  Result<vec_pair> simulate(double duration, double time_step = 0.01) override {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }
    if (time_step <= 0) {
      return Err(Error::InvalidArgument("Time step must be positive"));
    }

    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    // Initialize
    times[0] = 0.0;
    positions[0] = m_start_position;

    // Scale parameter for time step
    double scaled_sigma = m_sigma * std::sqrt(time_step);

    // Generate Cauchy increments (α = 1, β = 0)
    auto increments_result =
        rand_stable(num_steps, 1.0, 0.0, scaled_sigma, 0.0);
    if (!increments_result.has_value()) {
      return Err(increments_result.error());
    }
    auto increments = increments_result.value();

    // Simulate trajectory
    for (size_t i = 1; i <= num_steps; ++i) {
      times[i] = i * time_step;
      positions[i] = positions[i - 1] + increments[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

  /**
   * @brief Checks if the process has finite moments
   * @param order The moment order
   * @return False for all orders ≥ 1 (Cauchy distribution has no finite
   * moments)
   */
  [[nodiscard]] auto has_finite_moment(int order) const -> bool {
    return order < 1;
  }
};

/**
 * @brief Asymmetric Cauchy process implementation
 *
 * This class implements an asymmetric Cauchy process with skewness parameter β.
 * When β = 0, it reduces to the symmetric Cauchy process.
 */
export class AsymmetricCauchy : public ContinuousProcess {
  double m_beta = 0.0;           ///< Skewness parameter β ∈ [-1, 1]
  double m_sigma = 1.0;          ///< Scale parameter σ > 0
  double m_start_position = 0.0; ///< Initial position

public:
  /**
   * @brief Default constructor creating symmetric Cauchy process
   */
  AsymmetricCauchy() = default;

  /**
   * @brief Constructs asymmetric Cauchy process with specified parameters
   * @param beta Skewness parameter β (must be in [-1, 1])
   * @param sigma Scale parameter σ (must be positive)
   * @param start_position Initial position
   * @throws std::invalid_argument if parameters are invalid
   */
  AsymmetricCauchy(double beta, double sigma = 1.0, double start_position = 0.0)
      : m_beta(beta), m_sigma(sigma), m_start_position(start_position) {
    if (m_beta < -1.0 || m_beta > 1.0) {
      throw std::invalid_argument("Skewness parameter beta must be in [-1, 1]");
    }
    if (m_sigma <= 0.0) {
      throw std::invalid_argument("Scale parameter sigma must be positive");
    }
  }

  /**
   * @brief Gets the skewness parameter
   * @return The skewness parameter β
   */
  [[nodiscard]] auto get_beta() const -> double { return m_beta; }

  /**
   * @brief Gets the scale parameter
   * @return The scale parameter σ
   */
  [[nodiscard]] auto get_sigma() const -> double { return m_sigma; }

  /**
   * @brief Gets the initial position
   * @return The initial position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Simulates a trajectory of the asymmetric Cauchy process
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses independent asymmetric Cauchy increments:
   * X(t + dt) = X(t) + Stable(1, β, σ√dt, 0)
   */
  Result<vec_pair> simulate(double duration, double time_step = 0.01) override {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }
    if (time_step <= 0) {
      return Err(Error::InvalidArgument("Time step must be positive"));
    }

    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    // Initialize
    times[0] = 0.0;
    positions[0] = m_start_position;

    // Scale parameter for time step
    double scaled_sigma = m_sigma * std::sqrt(time_step);

    // Generate asymmetric Cauchy increments (α = 1)
    auto increments_result =
        rand_stable(num_steps, 1.0, m_beta, scaled_sigma, 0.0);
    if (!increments_result.has_value()) {
      return Err(increments_result.error());
    }
    auto increments = increments_result.value();

    // Simulate trajectory
    for (size_t i = 1; i <= num_steps; ++i) {
      times[i] = i * time_step;
      positions[i] = positions[i - 1] + increments[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

  /**
   * @brief Checks if the process has finite moments
   * @param order The moment order
   * @return False for all orders ≥ 1 (Cauchy distribution has no finite
   * moments)
   */
  [[nodiscard]] auto has_finite_moment(int order) const -> bool {
    return order < 1;
  }
};