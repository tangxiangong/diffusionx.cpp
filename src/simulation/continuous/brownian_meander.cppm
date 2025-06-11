module;

#include <algorithm>
#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.brownian_meander;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.continuous.brownian_bridge;

using std::vector;

/**
 * @brief Brownian meander implementation
 *
 * A Brownian meander is a Brownian motion conditioned to stay positive
 * over a finite time interval [0, T]. Unlike the excursion, it doesn't
 * necessarily end at 0.
 */
export class BrownianMeander : public ContinuousProcess {
  double m_total_time = 1.0; ///< Total time T

public:
  /**
   * @brief Default constructor creating a standard Brownian meander
   *
   * Creates a Brownian meander over time interval [0, 1].
   */
  BrownianMeander() = default;

  /**
   * @brief Constructs a Brownian meander with specified total time
   * @param total_time Total time T (must be positive)
   * @throws std::invalid_argument if total_time is not positive
   */
  explicit BrownianMeander(double total_time) : m_total_time(total_time) {
    if (m_total_time <= 0) {
      throw std::invalid_argument("Total time must be positive");
    }
  }

  /**
   * @brief Gets the total time
   * @return The total time T
   */
  [[nodiscard]] auto get_total_time() const -> double { return m_total_time; }

  /**
   * @brief Simulates a trajectory of the Brownian meander
   * @param duration The total simulation time (should equal total_time)
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses the reflection principle and rejection sampling.
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
          "Duration must equal total_time for Brownian meander"));
    }

    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    // Initialize time grid
    for (size_t i = 0; i <= num_steps; ++i) {
      times[i] = i * time_step;
    }

    // Use rejection sampling on Brownian motion
    const int max_attempts = 10000;

    for (int attempt = 0; attempt < max_attempts; ++attempt) {
      // Generate Brownian motion increments
      auto increments_result = randn(num_steps, 0.0, std::sqrt(time_step));
      if (!increments_result.has_value()) {
        return Err(increments_result.error());
      }
      auto increments = increments_result.value();

      // Build trajectory
      positions[0] = 0.0;
      bool all_positive = true;

      for (size_t i = 1; i <= num_steps; ++i) {
        positions[i] = positions[i - 1] + increments[i - 1];
        if (positions[i] <= 0) {
          all_positive = false;
          break;
        }
      }

      if (all_positive) {
        return Ok(std::make_pair(std::move(times), std::move(positions)));
      }
    }

    return Err(Error::SimulationFailed(
        "Failed to generate Brownian meander after maximum attempts"));
  }
};