module;

#include <algorithm>
#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.brownian_bridge;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Brownian bridge implementation
 *
 * A Brownian bridge is a continuous-time stochastic process B(t) that is
 * conditioned to start and end at specified values. Mathematically:
 *
 * B(t) | B(0) = a, B(T) = b
 *
 * The process has the property that:
 * E[B(t)] = a + (b - a) * t / T
 * Var[B(t)] = t * (T - t) / T
 */
export class BrownianBridge : public ContinuousProcess {
  double m_start_value = 0.0; ///< Starting value B(0)
  double m_end_value = 0.0;   ///< Ending value B(T)
  double m_total_time = 1.0;  ///< Total time T

public:
  /**
   * @brief Default constructor creating a standard Brownian bridge
   *
   * Creates a Brownian bridge from 0 to 0 over time interval [0, 1].
   */
  BrownianBridge() = default;

  /**
   * @brief Constructs a Brownian bridge with specified parameters
   * @param start_value Starting value B(0)
   * @param end_value Ending value B(T)
   * @param total_time Total time T (must be positive)
   * @throws std::invalid_argument if total_time is not positive
   */
  BrownianBridge(double start_value, double end_value, double total_time)
      : m_start_value(start_value), m_end_value(end_value),
        m_total_time(total_time) {
    if (m_total_time <= 0) {
      throw std::invalid_argument("Total time must be positive");
    }
  }

  /**
   * @brief Gets the starting value
   * @return The starting value B(0)
   */
  [[nodiscard]] auto get_start_value() const -> double { return m_start_value; }

  /**
   * @brief Gets the ending value
   * @return The ending value B(T)
   */
  [[nodiscard]] auto get_end_value() const -> double { return m_end_value; }

  /**
   * @brief Gets the total time
   * @return The total time T
   */
  [[nodiscard]] auto get_total_time() const -> double { return m_total_time; }

  /**
   * @brief Simulates a trajectory of the Brownian bridge
   * @param duration The total simulation time (should equal total_time)
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses the sequential conditioning method for exact simulation.
   */
  Result<vec_pair> simulate(double duration, double time_step = 0.01) override {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }
    if (time_step <= 0) {
      return Err(Error::InvalidArgument("Time step must be positive"));
    }
    if (std::abs(duration - m_total_time) > 1e-10) {
      return Err(Error::InvalidArgument(
          "Duration must equal total_time for Brownian bridge"));
    }

    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    // Initialize endpoints
    times[0] = 0.0;
    times[num_steps] = duration;
    positions[0] = m_start_value;
    positions[num_steps] = m_end_value;

    // Fill in time grid
    for (size_t i = 1; i < num_steps; ++i) {
      times[i] = i * time_step;
    }

    // Generate Brownian bridge using sequential conditioning
    auto result = generate_bridge_recursive(positions, times, 0, num_steps);
    if (!result.has_value()) {
      return Err(result.error());
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

  /**
   * @brief Computes the theoretical mean at time t
   * @param t Time point
   * @return The theoretical mean E[B(t)]
   */
  [[nodiscard]] auto theoretical_mean(double t) const -> double {
    if (t < 0 || t > m_total_time) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return m_start_value + (m_end_value - m_start_value) * t / m_total_time;
  }

  /**
   * @brief Computes the theoretical variance at time t
   * @param t Time point
   * @return The theoretical variance Var[B(t)]
   */
  [[nodiscard]] auto theoretical_variance(double t) const -> double {
    if (t < 0 || t > m_total_time) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return t * (m_total_time - t) / m_total_time;
  }

  /**
   * @brief Computes the covariance between B(s) and B(t)
   * @param s First time point
   * @param t Second time point
   * @return The covariance Cov[B(s), B(t)]
   */
  [[nodiscard]] auto theoretical_covariance(double s, double t) const
      -> double {
    if (s < 0 || s > m_total_time || t < 0 || t > m_total_time) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    double min_st = std::min(s, t);
    return min_st * (m_total_time - std::max(s, t)) / m_total_time;
  }

private:
  /**
   * @brief Recursively generates Brownian bridge using sequential conditioning
   * @param positions Position vector to fill
   * @param times Time vector
   * @param left_idx Left boundary index
   * @param right_idx Right boundary index
   * @return Result indicating success or error
   */
  Result<void> generate_bridge_recursive(vector<double> &positions,
                                         const vector<double> &times,
                                         size_t left_idx, size_t right_idx) {
    if (right_idx - left_idx <= 1) {
      return Ok(); // Base case: adjacent points
    }

    // Find midpoint
    size_t mid_idx = (left_idx + right_idx) / 2;
    double t_left = times[left_idx];
    double t_mid = times[mid_idx];
    double t_right = times[right_idx];
    double x_left = positions[left_idx];
    double x_right = positions[right_idx];

    // Conditional mean and variance for B(t_mid) | B(t_left), B(t_right)
    double alpha = (t_mid - t_left) / (t_right - t_left);
    double mean = x_left + alpha * (x_right - x_left);
    double variance = (t_mid - t_left) * (t_right - t_mid) / (t_right - t_left);

    // Generate random value
    auto noise_result = randn(1, 0.0, std::sqrt(variance));
    if (!noise_result.has_value()) {
      return Err(noise_result.error());
    }
    auto noise = noise_result.value();

    positions[mid_idx] = mean + noise[0];

    // Recursively fill left and right segments
    auto left_result =
        generate_bridge_recursive(positions, times, left_idx, mid_idx);
    if (!left_result.has_value()) {
      return Err(left_result.error());
    }

    auto right_result =
        generate_bridge_recursive(positions, times, mid_idx, right_idx);
    if (!right_result.has_value()) {
      return Err(right_result.error());
    }

    return Ok();
  }
};