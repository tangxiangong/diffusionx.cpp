module;

#include <cmath>
#include <optional>
#include <vector>

export module diffusionx.simulation.continuous.bm;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Brownian motion (Wiener process) implementation
 *
 * This class implements the standard Brownian motion, a continuous-time
 * stochastic process with independent, normally distributed increments. The
 * process satisfies:
 * - B(0) = start_position
 * - B(t) - B(s) ~ N(0, diffusion_coefficient * |t - s|) for t > s
 *
 * Mathematical definition:
 * dX(t) = √(2D) dW(t)
 *
 * where D is the diffusion coefficient and W(t) is a standard Wiener process.
 */
export class BrownianMotion : public ContinuousProcess {
  double m_start_position = 0.0;        ///< Initial position of the process
  double m_diffusion_coefficient = 1.0; ///< Diffusion coefficient (D)

public:
  /**
   * @brief Default constructor creating standard Brownian motion
   *
   * Creates a Brownian motion starting at position 0 with unit diffusion
   * coefficient.
   */
  BrownianMotion() = default;

  /**
   * @brief Constructs Brownian motion with specified parameters
   * @param start_position Initial position of the process
   * @param diffusion_coefficient Diffusion coefficient (must be positive)
   * @throws std::invalid_argument if diffusion_coefficient is not positive
   */
  BrownianMotion(double start_position, double diffusion_coefficient)
      : m_start_position(start_position),
        m_diffusion_coefficient(diffusion_coefficient) {
    if (m_diffusion_coefficient <= 0) {
      throw std::invalid_argument("Diffusion coefficient must be positive");
    }
  }

  /**
   * @brief Gets the initial position
   * @return The initial position of the process
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Gets the diffusion coefficient
   * @return The diffusion coefficient
   */
  [[nodiscard]] auto get_diffusion_coefficient() const -> double {
    return m_diffusion_coefficient;
  }

  /**
   * @brief Simulates a trajectory of the Brownian motion
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses the Euler-Maruyama scheme for simulation:
   * X(t + dt) = X(t) + √(2D * dt) * Z
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

    // Generate increments
    auto increments_result = randn(
        num_steps, 0.0, std::sqrt(2.0 * m_diffusion_coefficient * time_step));
    if (!increments_result.has_value()) {
      return Err(increments_result.error());
    }
    const auto& increments = increments_result.value();

    // Simulate trajectory
    for (size_t i = 1; i <= num_steps; ++i) {
      times[i] = i * time_step;
      positions[i] = positions[i - 1] + increments[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

  /**
   * @brief Computes the first passage time through a domain
   * @param domain The domain boundaries as a pair (lower, upper)
   * @param max_duration Maximum simulation time
   * @param time_step The time step for discretization
   * @return Result containing an optional FPT (None if no passage occurs), or
   * an Error
   */
  Result<Option<double>> fpt(double_pair domain, double max_duration = 1000,
                             double time_step = 0.01) override {
    auto [lower, upper] = domain;
    if (lower >= upper) {
      return Err(Error::InvalidArgument(
          "Invalid domain: lower bound must be less than upper bound"));
    }

    auto traj_result = simulate(max_duration, time_step);
    if (!traj_result.has_value()) {
      return Err(traj_result.error());
    }

    auto [times, positions] = traj_result.value();

    for (size_t i = 0; i < positions.size(); ++i) {
      if (positions[i] <= lower || positions[i] >= upper) {
        return Ok(Some(times[i]));
      }
    }

    return Ok(std::nullopt);
  }

  /**
   * @brief Computes the occupation time within a domain
   * @param domain The domain boundaries as a pair (lower, upper)
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing the occupation time, or an Error
   */
  Result<double> occupation_time(double_pair domain, double duration,
                                 double time_step = 0.01) override {
    auto [lower, upper] = domain;
    if (lower >= upper) {
      return Err(Error::InvalidArgument(
          "Invalid domain: lower bound must be less than upper bound"));
    }

    auto traj_result = simulate(duration, time_step);
    if (!traj_result.has_value()) {
      return Err(traj_result.error());
    }

    auto [times, positions] = traj_result.value();

    double occupation = 0.0;
    for (size_t i = 1; i < positions.size(); ++i) {
      if (positions[i] > lower && positions[i] < upper) {
        occupation += time_step;
      }
    }

    return Ok(occupation);
  }
};