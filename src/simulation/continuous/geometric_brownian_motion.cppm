module;

#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.geometric_brownian_motion;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Geometric Brownian Motion implementation
 *
 * This class implements the Geometric Brownian Motion, commonly used to model
 * stock prices and other financial instruments that cannot become negative.
 *
 * Mathematical definition:
 * dS(t) = μ S(t) dt + σ S(t) dW(t)
 *
 * Exact solution:
 * S(t) = S(0) * exp((μ - σ²/2)t + σ W(t))
 *
 * where:
 * - μ is the drift parameter
 * - σ > 0 is the volatility parameter
 * - W(t) is a standard Brownian motion
 * - S(0) is the initial value
 */
export class GeometricBrownianMotion : public ContinuousProcess {
  double m_start_position = 1.0; ///< Initial value S(0)
  double m_mu = 0.0;             ///< Drift parameter μ
  double m_sigma = 1.0;          ///< Volatility parameter σ

public:
  /**
   * @brief Default constructor creating standard GBM
   */
  GeometricBrownianMotion() = default;

  /**
   * @brief Constructs GBM with specified parameters
   * @param start_position Initial value S(0) (must be positive)
   * @param mu Drift parameter μ
   * @param sigma Volatility parameter σ (must be positive)
   * @throws std::invalid_argument if start_position or sigma is not positive
   */
  GeometricBrownianMotion(double start_position, double mu, double sigma)
      : m_start_position(start_position), m_mu(mu), m_sigma(sigma) {
    if (m_start_position <= 0) {
      throw std::invalid_argument("Initial value must be positive");
    }
    if (m_sigma <= 0) {
      throw std::invalid_argument("Volatility sigma must be positive");
    }
  }

  /**
   * @brief Gets the initial value
   * @return The initial value S(0)
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Gets the drift parameter
   * @return The drift parameter μ
   */
  [[nodiscard]] auto get_mu() const -> double { return m_mu; }

  /**
   * @brief Gets the volatility parameter
   * @return The volatility parameter σ
   */
  [[nodiscard]] auto get_sigma() const -> double { return m_sigma; }

  /**
   * @brief Simulates a trajectory of the GBM
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses the exact solution:
   * S(t + dt) = S(t) * exp((μ - σ²/2) * dt + σ * √dt * Z)
   * where Z ~ N(0, 1)
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

    // Precompute constants
    double drift_term = (m_mu - 0.5 * m_sigma * m_sigma) * time_step;
    double diffusion_term = m_sigma * std::sqrt(time_step);

    // Generate random increments
    auto increments_result = randn(num_steps, 0.0, 1.0);
    if (!increments_result.has_value()) {
      return Err(increments_result.error());
    }
    auto increments = increments_result.value();

    // Simulate trajectory using exact solution
    for (size_t i = 1; i <= num_steps; ++i) {
      times[i] = i * time_step;
      double log_increment = drift_term + diffusion_term * increments[i - 1];
      positions[i] = positions[i - 1] * std::exp(log_increment);
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

  /**
   * @brief Computes the theoretical mean at time t
   * @param t Time point
   * @return The theoretical mean E[S(t)]
   */
  [[nodiscard]] auto theoretical_mean(double t) const -> double {
    return m_start_position * std::exp(m_mu * t);
  }

  /**
   * @brief Computes the theoretical variance at time t
   * @param t Time point
   * @return The theoretical variance Var[S(t)]
   */
  [[nodiscard]] auto theoretical_variance(double t) const -> double {
    double mean_val = theoretical_mean(t);
    return mean_val * mean_val * (std::exp(m_sigma * m_sigma * t) - 1.0);
  }
};