module;

#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.gamma;

import diffusionx.error;
import diffusionx.random.gamma;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Gamma process implementation
 *
 * A Gamma process is a non-negative, non-decreasing Lévy process with
 * independent Gamma-distributed increments. It is commonly used to model
 * degradation processes and cumulative damage.
 *
 * Mathematical definition:
 * X(t) has independent increments with X(t) - X(s) ~ Gamma(α(t-s), β) for t > s
 *
 * where:
 * - α > 0 is the shape parameter (rate of events)
 * - β > 0 is the rate parameter (inverse scale)
 *
 * Properties:
 * - E[X(t)] = αt/β
 * - Var[X(t)] = αt/β²
 * - X(t) is non-decreasing and non-negative
 */
export class GammaProcess : public ContinuousProcess {
  double m_shape = 1.0; ///< Shape parameter α > 0
  double m_rate = 1.0;  ///< Rate parameter β > 0

public:
  /**
   * @brief Default constructor creating standard Gamma process
   */
  GammaProcess() = default;

  /**
   * @brief Constructs Gamma process with specified parameters
   * @param shape Shape parameter α (must be positive)
   * @param rate Rate parameter β (must be positive)
   * @throws std::invalid_argument if parameters are not positive
   */
  GammaProcess(double shape, double rate) : m_shape(shape), m_rate(rate) {
    if (m_shape <= 0.0) {
      throw std::invalid_argument("Shape parameter must be positive");
    }
    if (m_rate <= 0.0) {
      throw std::invalid_argument("Rate parameter must be positive");
    }
  }

  /**
   * @brief Gets the shape parameter
   * @return The shape parameter α
   */
  [[nodiscard]] auto get_shape() const -> double { return m_shape; }

  /**
   * @brief Gets the rate parameter
   * @return The rate parameter β
   */
  [[nodiscard]] auto get_rate() const -> double { return m_rate; }

  /**
   * @brief Simulates a trajectory of the Gamma process
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses independent Gamma increments:
   * X(t + dt) = X(t) + Gamma(α * dt, β)
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
    positions[0] = 0.0;

    // Generate Gamma increments
    double scale = 1.0 / m_rate;
    auto increments_result = rand_gamma(num_steps, m_shape * time_step, scale);
    if (!increments_result.has_value()) {
      return Err(increments_result.error());
    }
    auto increments = increments_result.value();

    // Simulate trajectory (cumulative sum)
    for (size_t i = 1; i <= num_steps; ++i) {
      times[i] = i * time_step;
      positions[i] = positions[i - 1] + increments[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

  /**
   * @brief Computes the theoretical mean at time t
   * @param t Time point
   * @return The theoretical mean E[X(t)]
   */
  [[nodiscard]] auto theoretical_mean(double t) const -> double {
    return m_shape * t / m_rate;
  }

  /**
   * @brief Computes the theoretical variance at time t
   * @param t Time point
   * @return The theoretical variance Var[X(t)]
   */
  [[nodiscard]] auto theoretical_variance(double t) const -> double {
    return m_shape * t / (m_rate * m_rate);
  }
};