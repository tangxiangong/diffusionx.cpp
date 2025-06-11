module;

#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.bng;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Brownian yet non-Gaussian (BnG) process implementation
 *
 * The BnG process is a stochastic process that exhibits Brownian scaling
 * (mean square displacement ~ t) but has non-Gaussian displacement
 * distributions. This is achieved by coupling a diffusion process with a
 * time-varying diffusion coefficient driven by an Ornstein-Uhlenbeck process.
 *
 * Mathematical definition:
 * dr(t) = √(2D(t)) dW₁(t), r(0) = r₀
 * D(t) = Y(t)²
 * dY(t) = -Y(t) dt + dW₂(t), Y(0) = Y₀
 *
 * where W₁(t) and W₂(t) are two independent Wiener processes.
 *
 * Properties:
 * - Linear mean square displacement: ⟨r²(t)⟩ ~ t
 * - Non-Gaussian displacement distributions
 * - Exponential tails in displacement probability density
 */
export class BrownianNonGaussian : public ContinuousProcess {
  double m_start_position = 0.0;    ///< Initial position r₀
  double m_ou_start_position = 1.0; ///< Initial OU process value Y₀

public:
  /**
   * @brief Default constructor creating standard BnG process
   */
  BrownianNonGaussian() = default;

  /**
   * @brief Constructs BnG process with specified parameters
   * @param start_position Initial position r₀
   * @param ou_start_position Initial OU process value Y₀
   */
  BrownianNonGaussian(double start_position, double ou_start_position)
      : m_start_position(start_position),
        m_ou_start_position(ou_start_position) {}

  /**
   * @brief Gets the initial position
   * @return The initial position r₀
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Gets the initial OU process value
   * @return The initial OU process value Y₀
   */
  [[nodiscard]] auto get_ou_start_position() const -> double {
    return m_ou_start_position;
  }

  /**
   * @brief Simulates a trajectory of the BnG process
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Algorithm:
   * 1. Simulate OU process Y(t) with θ = 1, σ = 1
   * 2. Use |Y(t)| as time-varying diffusion coefficient
   * 3. Generate position increments with √(2|Y(t)|dt) * Z
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

    // Simulate OU process Y(t) with θ = 1, σ = 1
    auto ou_result = simulate_ou_process(duration, time_step);
    if (!ou_result.has_value()) {
      return Err(ou_result.error());
    }
    auto ou_trajectory = ou_result.value();

    // Generate Brownian increments
    auto noise_result = randn(num_steps, 0.0, 1.0);
    if (!noise_result.has_value()) {
      return Err(noise_result.error());
    }
    auto noise = noise_result.value();

    // Simulate BnG trajectory
    for (size_t i = 1; i <= num_steps; ++i) {
      times[i] = i * time_step;

      // Time-varying diffusion coefficient D(t) = |Y(t)|
      double diffusion_coeff = std::abs(ou_trajectory[i]);

      // Position increment: √(2D(t)dt) * Z
      double increment =
          std::sqrt(2.0 * diffusion_coeff * time_step) * noise[i - 1];
      positions[i] = positions[i - 1] + increment;
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

private:
  /**
   * @brief Simulates the OU process Y(t) with θ = 1, σ = 1
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing OU trajectory, or an Error
   */
  Result<vector<double>> simulate_ou_process(double duration,
                                             double time_step) {
    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> y_trajectory(num_steps + 1);

    // Initialize OU process
    y_trajectory[0] = m_ou_start_position;

    // OU parameters: θ = 1, σ = 1
    double theta = 1.0;
    double sigma = 1.0;

    // Precompute constants for exact discretization
    double exp_theta_dt = std::exp(-theta * time_step);
    double mean_coeff = 1.0 - exp_theta_dt;
    double var_coeff =
        sigma *
        std::sqrt((1.0 - std::exp(-2.0 * theta * time_step)) / (2.0 * theta));

    // Generate random increments for OU process
    auto ou_noise_result = randn(num_steps, 0.0, 1.0);
    if (!ou_noise_result.has_value()) {
      return Err(ou_noise_result.error());
    }
    auto ou_noise = ou_noise_result.value();

    // Simulate OU trajectory using exact discretization
    for (size_t i = 1; i <= num_steps; ++i) {
      y_trajectory[i] =
          y_trajectory[i - 1] * exp_theta_dt + var_coeff * ou_noise[i - 1];
    }

    return Ok(std::move(y_trajectory));
  }
};