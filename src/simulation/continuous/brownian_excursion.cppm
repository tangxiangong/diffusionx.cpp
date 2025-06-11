module;

#include <algorithm>
#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.brownian_excursion;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.continuous.brownian_bridge;

using std::vector;

/**
 * @brief Brownian excursion implementation
 *
 * A Brownian excursion is a Brownian bridge conditioned to be positive
 * throughout the interval (0, T). This is a special case of the Brownian bridge
 * where both endpoints are 0 and the process stays positive.
 */
export class BrownianExcursion : public ContinuousProcess {
  double m_total_time = 1.0; ///< Total time T

public:
  /**
   * @brief Default constructor creating a standard Brownian excursion
   *
   * Creates a Brownian excursion over time interval [0, 1].
   */
  BrownianExcursion() = default;

  /**
   * @brief Constructs a Brownian excursion with specified total time
   * @param total_time Total time T (must be positive)
   * @throws std::invalid_argument if total_time is not positive
   */
  explicit BrownianExcursion(double total_time) : m_total_time(total_time) {
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
   * @brief Simulates a trajectory of the Brownian excursion
   * @param duration The total simulation time (should equal total_time)
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses rejection sampling on Brownian bridges.
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
          "Duration must equal total_time for Brownian excursion"));
    }

    // Use rejection sampling
    BrownianBridge bridge(0.0, 0.0, m_total_time);
    const int max_attempts = 10000;

    for (int attempt = 0; attempt < max_attempts; ++attempt) {
      auto bridge_result = bridge.simulate(duration, time_step);
      if (!bridge_result.has_value()) {
        return Err(bridge_result.error());
      }

      auto [times, positions] = bridge_result.value();

      // Check if all interior points are positive
      bool all_positive = true;
      for (size_t i = 1; i < positions.size() - 1; ++i) {
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
        "Failed to generate Brownian excursion after maximum attempts"));
  }
};
