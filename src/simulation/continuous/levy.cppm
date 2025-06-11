module;

#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.levy;

import diffusionx.error;
import diffusionx.random.stable;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Lévy process implementation using stable distributions
 *
 * This class implements a Lévy process with stable increments, which includes
 * Brownian motion as a special case when α = 2.
 *
 * Mathematical definition:
 * X(t) = X(0) + L(t)
 *
 * where L(t) is a Lévy process with independent, stationary increments
 * following a stable distribution with parameters (α, β, σ, μ).
 */
export class Levy : public ContinuousProcess {
  double m_alpha = 2.0;          ///< Stability parameter α ∈ (0, 2]
  double m_beta = 0.0;           ///< Skewness parameter β ∈ [-1, 1]
  double m_sigma = 1.0;          ///< Scale parameter σ > 0
  double m_mu = 0.0;             ///< Location parameter μ
  double m_start_position = 0.0; ///< Initial position

public:
  /**
   * @brief Default constructor creating standard Brownian motion (α = 2, β = 0)
   */
  Levy() = default;

  /**
   * @brief Constructs Lévy process with specified parameters
   * @param alpha Stability parameter α (must be in (0, 2])
   * @param beta Skewness parameter β (must be in [-1, 1])
   * @param sigma Scale parameter σ (must be positive)
   * @param mu Location parameter μ
   * @param start_position Initial position
   * @throws std::invalid_argument if parameters are invalid
   */
  Levy(double alpha, double beta, double sigma, double mu,
       double start_position = 0.0)
      : m_alpha(alpha), m_beta(beta), m_sigma(sigma), m_mu(mu),
        m_start_position(start_position) {
    if (m_alpha <= 0.0 || m_alpha > 2.0) {
      throw std::invalid_argument(
          "Stability parameter alpha must be in (0, 2]");
    }
    if (m_beta < -1.0 || m_beta > 1.0) {
      throw std::invalid_argument("Skewness parameter beta must be in [-1, 1]");
    }
    if (m_sigma <= 0.0) {
      throw std::invalid_argument("Scale parameter sigma must be positive");
    }
  }

  /**
   * @brief Gets the stability parameter
   * @return The stability parameter α
   */
  [[nodiscard]] auto get_alpha() const -> double { return m_alpha; }

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
   * @brief Gets the location parameter
   * @return The location parameter μ
   */
  [[nodiscard]] auto get_mu() const -> double { return m_mu; }

  /**
   * @brief Gets the initial position
   * @return The initial position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Simulates a trajectory of the Lévy process
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses independent stable increments:
   * X(t + dt) = X(t) + S(α, β, σ * dt^(1/α), μ * dt)
   * where S denotes a stable random variable
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

    // Scale parameters for time step
    double scale_factor = std::pow(time_step, 1.0 / m_alpha);
    double scaled_sigma = m_sigma * scale_factor;
    double scaled_mu = m_mu * time_step;

    // Generate stable increments
    auto increments_result =
        rand_stable(num_steps, m_alpha, m_beta, scaled_sigma, scaled_mu);
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
   * @brief Computes the theoretical mean at time t (when it exists)
   * @param t Time point
   * @return The theoretical mean E[X(t)] if α > 1, otherwise NaN
   */
  [[nodiscard]] auto theoretical_mean(double t) const -> double {
    if (m_alpha > 1.0) {
      return m_start_position + m_mu * t;
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

  /**
   * @brief Checks if the process has finite variance
   * @return True if α = 2, false otherwise
   */
  [[nodiscard]] auto has_finite_variance() const -> bool {
    return std::abs(m_alpha - 2.0) < 1e-10;
  }
};

/**
 * @brief Symmetric α-stable Lévy process (β = 0)
 *
 * This is a special case of the Lévy process with zero skewness,
 * commonly used in financial modeling and physics.
 */
export class SymmetricLevyProcess : public Levy {
public:
  /**
   * @brief Constructs symmetric Lévy process
   * @param alpha Stability parameter α (must be in (0, 2])
   * @param sigma Scale parameter σ (must be positive)
   * @param start_position Initial position
   */
  SymmetricLevyProcess(double alpha, double sigma = 1.0,
                       double start_position = 0.0)
      : Levy(alpha, 0.0, sigma, 0.0, start_position) {}
};
