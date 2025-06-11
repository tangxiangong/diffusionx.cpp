module;

#include <cmath>
#include <random>
#include <vector>

export module diffusionx.simulation.point.ctrw;

import diffusionx.error;
import diffusionx.random.stable;
import diffusionx.random.normal;
import diffusionx.random.exponential;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.basic.abstract;

using std::vector;

/**
 * @brief Continuous Time Random Walk (CTRW) implementation
 *
 * A continuous time random walk (CTRW) is a stochastic process that generalizes
 * random walks by introducing random waiting times between jumps.
 * Mathematically:
 *
 * X(t) = ∑_{i=1}^{N(t)} J_i
 *
 * where:
 * - X(t) is the position at time t
 * - J_i are random jump lengths (often from a symmetric distribution)
 * - N(t) is a counting process representing the number of jumps by time t
 *
 * The waiting times between jumps typically follow a distribution with heavy
 * tails, often characterized by a power-law. When the waiting time distribution
 * has infinite mean, the resulting process exhibits subdiffusive behavior.
 */
export class CTRW {
  double m_alpha = 1.0;          ///< Waiting time distribution exponent
  double m_beta = 2.0;           ///< Jump size distribution exponent
  double m_start_position = 0.0; ///< Starting position

public:
  /**
   * @brief Default constructor creating a standard CTRW
   */
  CTRW() = default;

  /**
   * @brief Constructs a CTRW with specified parameters
   * @param alpha Waiting time distribution exponent (0 < α ≤ 1)
   * @param beta Jump size distribution exponent (0 < β ≤ 2)
   * @param start_position Starting position
   * @throws std::invalid_argument if parameters are invalid
   */
  CTRW(double alpha, double beta, double start_position = 0.0)
      : m_alpha(alpha), m_beta(beta), m_start_position(start_position) {
    if (m_alpha <= 0.0 || m_alpha > 1.0) {
      throw std::invalid_argument("Alpha must be in range (0, 1]");
    }
    if (m_beta <= 0.0 || m_beta > 2.0) {
      throw std::invalid_argument("Beta must be in range (0, 2]");
    }
  }

  /**
   * @brief Gets the waiting time distribution exponent
   * @return The exponent α
   */
  [[nodiscard]] auto get_alpha() const -> double { return m_alpha; }

  /**
   * @brief Gets the jump size distribution exponent
   * @return The exponent β
   */
  [[nodiscard]] auto get_beta() const -> double { return m_beta; }

  /**
   * @brief Gets the starting position
   * @return The starting position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Simulates the CTRW with a given number of steps
   * @param num_steps The number of steps to simulate
   * @return Result containing time and position vectors, or an Error
   */
  Result<vec_pair> simulate_with_steps(size_t num_steps) {
    if (num_steps == 0) {
      return Err(Error::InvalidArgument("Number of steps must be positive"));
    }

    // Generate waiting times
    vector<double> waiting_times;
    if (m_alpha == 1.0) {
      // Exponential distribution for α = 1
      auto exp_result = randexp(num_steps, 1.0);
      if (!exp_result.has_value()) {
        return Err(exp_result.error());
      }
      waiting_times = exp_result.value();
    } else {
      // Stable distribution for α < 1
      auto stable_result = rand_stable(num_steps, m_alpha, 1.0, 1.0, 0.0);
      if (!stable_result.has_value()) {
        return Err(stable_result.error());
      }
      waiting_times = stable_result.value();

      // Ensure positive waiting times
      for (auto &wt : waiting_times) {
        wt = std::abs(wt);
      }
    }

    // Generate jump lengths
    vector<double> jump_lengths;
    if (m_beta == 2.0) {
      // Normal distribution for β = 2
      auto normal_result = randn(num_steps, 0.0, 1.0);
      if (!normal_result.has_value()) {
        return Err(normal_result.error());
      }
      jump_lengths = normal_result.value();
    } else {
      // Stable distribution for β < 2
      auto stable_result = rand_stable(num_steps, m_beta, 0.0, 1.0, 0.0);
      if (!stable_result.has_value()) {
        return Err(stable_result.error());
      }
      jump_lengths = stable_result.value();
    }

    // Build trajectory
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    times[0] = 0.0;
    positions[0] = m_start_position;

    for (size_t i = 1; i <= num_steps; ++i) {
      times[i] = times[i - 1] + waiting_times[i - 1];
      positions[i] = positions[i - 1] + jump_lengths[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

  /**
   * @brief Simulates the CTRW with a given duration
   * @param duration The total simulation time
   * @return Result containing time and position vectors, or an Error
   */
  Result<vec_pair> simulate_with_duration(double duration) {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }

    // Start with an estimate of the number of steps needed
    size_t num_steps = static_cast<size_t>(std::ceil(duration));

    while (true) {
      auto result = simulate_with_steps(num_steps);
      if (!result.has_value()) {
        return Err(result.error());
      }

      auto [times, positions] = result.value();

      if (times.back() >= duration) {
        // Find the index where time exceeds duration
        auto it = std::upper_bound(times.begin(), times.end(), duration);
        size_t index = std::distance(times.begin(), it);

        // Truncate to the desired duration
        vector<double> truncated_times(times.begin(), times.begin() + index);
        vector<double> truncated_positions(positions.begin(),
                                           positions.begin() + index);

        // Add final point at exact duration
        if (index > 0 && truncated_times.back() < duration) {
          truncated_times.push_back(duration);
          truncated_positions.push_back(truncated_positions.back());
        }

        return Ok(std::make_pair(std::move(truncated_times),
                                 std::move(truncated_positions)));
      }

      // Need more steps
      num_steps *= 2;
    }
  }

  /**
   * @brief Simulates the CTRW and interpolates to regular time grid
   * @param duration The total simulation time
   * @param time_step The time step for interpolation
   * @return Result containing time and position vectors, or an Error
   */
  Result<vec_pair> simulate(double duration, double time_step = 0.01) {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }
    if (time_step <= 0) {
      return Err(Error::InvalidArgument("Time step must be positive"));
    }

    // First simulate with duration
    auto trajectory_result = simulate_with_duration(duration);
    if (!trajectory_result.has_value()) {
      return Err(trajectory_result.error());
    }

    auto [event_times, event_positions] = trajectory_result.value();

    // Interpolate to regular time grid
    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    for (size_t i = 0; i <= num_steps; ++i) {
      times[i] = i * time_step;

      // Find the position at this time using piecewise constant interpolation
      auto it =
          std::upper_bound(event_times.begin(), event_times.end(), times[i]);
      if (it == event_times.begin()) {
        positions[i] = m_start_position;
      } else {
        size_t index = std::distance(event_times.begin(), it) - 1;
        positions[i] = event_positions[index];
      }
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }
};