module;

#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.ou;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Ornstein-Uhlenbeck process implementation
 *
 * This class implements the Ornstein-Uhlenbeck process, a mean-reverting
 * stochastic process commonly used to model interest rates, volatility, and
 * other financial quantities.
 *
 * Mathematical definition:
 * dX(t) = θ(μ - X(t))dt + σ dW(t)
 *
 * where:
 * - θ > 0 is the mean reversion speed
 * - μ is the long-term mean
 * - σ > 0 is the volatility parameter
 * - W(t) is a standard Brownian motion
 */
export class OrnsteinUhlenbeck : public ContinuousProcess {
  double m_theta = 1.0;          ///< Mean reversion speed
  double m_mu = 0.0;             ///< Long-term mean
  double m_sigma = 1.0;          ///< Volatility parameter
  double m_start_position = 0.0; ///< Initial position

public:
  /**
   * @brief Default constructor creating standard OU process
   */
  OrnsteinUhlenbeck() = default;

  /**
   * @brief Constructs OU process with specified parameters
   * @param theta Mean reversion speed (must be positive)
   * @param mu Long-term mean
   * @param sigma Volatility parameter (must be positive)
   * @param start_position Initial position
   * @throws std::invalid_argument if theta or sigma is not positive
   */
  OrnsteinUhlenbeck(double theta, double mu, double sigma,
                    double start_position)
      : m_theta(theta), m_mu(mu), m_sigma(sigma),
        m_start_position(start_position) {
    if (m_theta <= 0) {
      throw std::invalid_argument(
          "Mean reversion speed theta must be positive");
    }
    if (m_sigma <= 0) {
      throw std::invalid_argument("Volatility sigma must be positive");
    }
  }

  /**
   * @brief Gets the mean reversion speed
   * @return The mean reversion speed θ
   */
  [[nodiscard]] auto get_theta() const -> double { return m_theta; }

  /**
   * @brief Gets the long-term mean
   * @return The long-term mean μ
   */
  [[nodiscard]] auto get_mu() const -> double { return m_mu; }

  /**
   * @brief Gets the volatility parameter
   * @return The volatility parameter σ
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
   * @brief Simulates a trajectory of the OU process
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses the exact solution for discretization:
   * X(t + dt) = X(t) * exp(-θ * dt) + μ * (1 - exp(-θ * dt)) + σ * √((1 -
   * exp(-2θ * dt)) / (2θ)) * Z where Z ~ N(0, 1)
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

    // Precompute constants for exact discretization
    double exp_theta_dt = std::exp(-m_theta * time_step);
    double mean_coeff = 1.0 - exp_theta_dt;
    double var_coeff =
        m_sigma * std::sqrt((1.0 - std::exp(-2.0 * m_theta * time_step)) /
                            (2.0 * m_theta));

    // Generate random increments
    auto increments_result = randn(num_steps, 0.0, 1.0);
    if (!increments_result.has_value()) {
      return Err(increments_result.error());
    }
    auto increments = increments_result.value();

    // Simulate trajectory using exact discretization
    for (size_t i = 1; i <= num_steps; ++i) {
      times[i] = i * time_step;
      positions[i] = positions[i - 1] * exp_theta_dt + m_mu * mean_coeff +
                     var_coeff * increments[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

  /**
   * @brief Computes the theoretical mean at time t
   * @param t Time point
   * @return The theoretical mean E[X(t)]
   */
  [[nodiscard]] auto theoretical_mean(double t) const -> double {
    return m_mu + (m_start_position - m_mu) * std::exp(-m_theta * t);
  }

  /**
   * @brief Computes the theoretical variance at time t
   * @param t Time point
   * @return The theoretical variance Var[X(t)]
   */
  [[nodiscard]] auto theoretical_variance(double t) const -> double {
    return (m_sigma * m_sigma / (2.0 * m_theta)) *
           (1.0 - std::exp(-2.0 * m_theta * t));
  }
};