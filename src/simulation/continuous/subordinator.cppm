module;

#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.subordinator;

import diffusionx.error;
import diffusionx.random.stable;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Subordinator process implementation
 *
 * A subordinator is a non-decreasing Lévy process, commonly used to model
 * random time changes in stochastic processes. This implementation uses
 * a stable subordinator with index α ∈ (0, 1).
 *
 * Mathematical properties:
 * - S(0) = 0
 * - S(t) is non-decreasing
 * - S(t) has independent, stationary increments
 * - S(t + s) - S(t) ~ Stable(α, 1, s^(1/α), 0) for α ∈ (0, 1)
 */
export class Subordinator : public ContinuousProcess {
  double m_alpha = 0.5; ///< Stability index α ∈ (0, 1)

public:
  /**
   * @brief Default constructor creating subordinator with α = 0.5
   */
  Subordinator() = default;

  /**
   * @brief Constructs subordinator with specified stability index
   * @param alpha Stability index α (must be in (0, 1))
   * @throws std::invalid_argument if alpha is not in (0, 1)
   */
  explicit Subordinator(double alpha) : m_alpha(alpha) {
    if (m_alpha <= 0.0 || m_alpha >= 1.0) {
      throw std::invalid_argument("Stability index alpha must be in (0, 1)");
    }
  }

  /**
   * @brief Gets the stability index
   * @return The stability index α
   */
  [[nodiscard]] auto get_alpha() const -> double { return m_alpha; }

  /**
   * @brief Simulates a trajectory of the subordinator
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses the fact that increments follow a maximally skewed stable
   * distribution: S(t + dt) - S(t) ~ Stable(α, 1, dt^(1/α), 0)
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

    // Scale parameter for time step
    double scale_factor = std::pow(time_step, 1.0 / m_alpha);

    // Generate maximally skewed stable increments
    auto increments_result = rand_skew_stable(num_steps, m_alpha);
    if (!increments_result.has_value()) {
      return Err(increments_result.error());
    }
    auto increments = increments_result.value();

    // Simulate trajectory (cumulative sum of scaled increments)
    for (size_t i = 1; i <= num_steps; ++i) {
      times[i] = i * time_step;
      positions[i] = positions[i - 1] + scale_factor * increments[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

  /**
   * @brief Computes the theoretical mean at time t
   * @param t Time point
   * @return The theoretical mean E[S(t)]
   */
  [[nodiscard]] auto theoretical_mean(double t) const -> double {
    // For maximally skewed stable subordinator, the mean grows as t
    return t;
  }
};

/**
 * @brief Inverse subordinator process
 *
 * The inverse subordinator T(t) is defined as the first passage time
 * of the subordinator S(u) to level t:
 * T(t) = inf{u ≥ 0 : S(u) > t}
 *
 * This process is used to model random time changes and is the inverse
 * of the subordinator process.
 */
export class InverseSubordinator : public ContinuousProcess {
  double m_alpha = 0.5; ///< Stability index α ∈ (0, 1)

public:
  /**
   * @brief Default constructor creating inverse subordinator with α = 0.5
   */
  InverseSubordinator() = default;

  /**
   * @brief Constructs inverse subordinator with specified stability index
   * @param alpha Stability index α (must be in (0, 1))
   * @throws std::invalid_argument if alpha is not in (0, 1)
   */
  explicit InverseSubordinator(double alpha) : m_alpha(alpha) {
    if (m_alpha <= 0.0 || m_alpha >= 1.0) {
      throw std::invalid_argument("Stability index alpha must be in (0, 1)");
    }
  }

  /**
   * @brief Gets the stability index
   * @return The stability index α
   */
  [[nodiscard]] auto get_alpha() const -> double { return m_alpha; }

  /**
   * @brief Simulates a trajectory of the inverse subordinator
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses the relationship between subordinator and its inverse through
   * first passage times.
   */
  Result<vec_pair> simulate(double duration, double time_step = 0.01) override {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }
    if (time_step <= 0) {
      return Err(Error::InvalidArgument("Time step must be positive"));
    }

    // First, simulate a subordinator with fine time resolution
    double fine_time_step = time_step / 10.0;
    Subordinator subordinator(m_alpha);

    // Simulate subordinator up to a large time to ensure we cover the duration
    double max_time = duration * 10.0; // Heuristic upper bound
    auto sub_result = subordinator.simulate(max_time, fine_time_step);
    if (!sub_result.has_value()) {
      return Err(sub_result.error());
    }

    auto [sub_times, sub_positions] = sub_result.value();

    // Create inverse subordinator by finding first passage times
    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    times[0] = 0.0;
    positions[0] = 0.0;

    size_t sub_index = 0;
    for (size_t i = 1; i <= num_steps; ++i) {
      double target_level = i * time_step;
      times[i] = target_level;

      // Find first passage time to level target_level
      while (sub_index < sub_positions.size() &&
             sub_positions[sub_index] <= target_level) {
        ++sub_index;
      }

      if (sub_index < sub_times.size()) {
        positions[i] = sub_times[sub_index];
      } else {
        // If we haven't reached the target level, extrapolate
        positions[i] =
            positions[i - 1] + time_step / std::pow(target_level, m_alpha);
      }
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }
};