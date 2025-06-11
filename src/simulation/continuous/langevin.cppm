module;

#include <cmath>
#include <functional>
#include <random>
#include <vector>

export module diffusionx.simulation.continuous.langevin;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.random.stable;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::function;
using std::vector;

/**
 * @brief Langevin equation implementation
 *
 * This class implements the Langevin equation:
 * dx(t) = f(x(t), t) dt + g(x(t), t) dW(t), x(0) = x0
 *
 * where:
 * - f(x, t) is the drift function
 * - g(x, t) is the diffusion function
 * - W(t) is the Wiener process (Brownian motion)
 */
export template <typename DriftFunc, typename DiffusionFunc>
class Langevin : public ContinuousProcess {
  static_assert(std::is_invocable_r_v<double, DriftFunc, double, double>,
                "DriftFunc must be callable with (double, double) -> double");
  static_assert(
      std::is_invocable_r_v<double, DiffusionFunc, double, double>,
      "DiffusionFunc must be callable with (double, double) -> double");

private:
  DriftFunc m_drift_func;         ///< Drift function f(x, t)
  DiffusionFunc m_diffusion_func; ///< Diffusion function g(x, t)
  double m_start_position = 0.0;  ///< Initial position

public:
  /**
   * @brief Constructs a Langevin equation
   * @param drift_func The drift function f(x, t)
   * @param diffusion_func The diffusion function g(x, t)
   * @param start_position Initial position
   */
  Langevin(DriftFunc drift_func, DiffusionFunc diffusion_func,
           double start_position = 0.0)
      : m_drift_func(std::move(drift_func)),
        m_diffusion_func(std::move(diffusion_func)),
        m_start_position(start_position) {}

  /**
   * @brief Gets the initial position
   * @return The initial position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Gets the drift function
   * @return Reference to the drift function
   */
  [[nodiscard]] auto get_drift_func() const -> const DriftFunc & {
    return m_drift_func;
  }

  /**
   * @brief Gets the diffusion function
   * @return Reference to the diffusion function
   */
  [[nodiscard]] auto get_diffusion_func() const -> const DiffusionFunc & {
    return m_diffusion_func;
  }

  /**
   * @brief Simulates a trajectory of the Langevin equation
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses the Euler-Maruyama scheme:
   * X(t + dt) = X(t) + f(X(t), t) * dt + g(X(t), t) * sqrt(dt) * Z
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

    // Generate random increments
    auto noise_result = randn(num_steps, 0.0, 1.0);
    if (!noise_result.has_value()) {
      return Err(noise_result.error());
    }
    auto noise = noise_result.value();

    double sqrt_dt = std::sqrt(time_step);

    // Simulate trajectory using Euler-Maruyama scheme
    for (size_t i = 1; i <= num_steps; ++i) {
      double t = (i - 1) * time_step;
      double x = positions[i - 1];

      times[i] = i * time_step;
      positions[i] = x + m_drift_func(x, t) * time_step +
                     m_diffusion_func(x, t) * sqrt_dt * noise[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }
};

/**
 * @brief Generalized Langevin equation implementation
 *
 * This class implements the generalized Langevin equation:
 * dx(t) = f(x(t), t) dt + g(x(t), t) dL_α(t), x(0) = x0
 *
 * where:
 * - f(x, t) is the drift function
 * - g(x, t) is the diffusion function
 * - L_α(t) is the α-stable process
 */
export template <typename DriftFunc, typename DiffusionFunc>
class GeneralizedLangevin : public ContinuousProcess {
  static_assert(std::is_invocable_r_v<double, DriftFunc, double, double>,
                "DriftFunc must be callable with (double, double) -> double");
  static_assert(
      std::is_invocable_r_v<double, DiffusionFunc, double, double>,
      "DiffusionFunc must be callable with (double, double) -> double");

private:
  DriftFunc m_drift_func;         ///< Drift function f(x, t)
  DiffusionFunc m_diffusion_func; ///< Diffusion function g(x, t)
  double m_start_position = 0.0;  ///< Initial position
  double m_alpha = 1.5;           ///< Stability index

public:
  /**
   * @brief Constructs a generalized Langevin equation
   * @param drift_func The drift function f(x, t)
   * @param diffusion_func The diffusion function g(x, t)
   * @param start_position Initial position
   * @param alpha Stability index (0 < α ≤ 2)
   */
  GeneralizedLangevin(DriftFunc drift_func, DiffusionFunc diffusion_func,
                      double start_position = 0.0, double alpha = 1.5)
      : m_drift_func(std::move(drift_func)),
        m_diffusion_func(std::move(diffusion_func)),
        m_start_position(start_position), m_alpha(alpha) {
    if (alpha <= 0.0 || alpha > 2.0) {
      throw std::invalid_argument("Alpha must be in range (0, 2]");
    }
  }

  /**
   * @brief Gets the initial position
   * @return The initial position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Gets the stability index
   * @return The stability index
   */
  [[nodiscard]] auto get_alpha() const -> double { return m_alpha; }

  /**
   * @brief Simulates a trajectory of the generalized Langevin equation
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
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

    // Generate stable increments
    auto noise_result = rand_stable(num_steps, m_alpha, 0.0, 1.0, 0.0);
    if (!noise_result.has_value()) {
      return Err(noise_result.error());
    }
    auto noise = noise_result.value();

    double dt_alpha = std::pow(time_step, 1.0 / m_alpha);

    // Simulate trajectory
    for (size_t i = 1; i <= num_steps; ++i) {
      double t = (i - 1) * time_step;
      double x = positions[i - 1];

      times[i] = i * time_step;
      positions[i] = x + m_drift_func(x, t) * time_step +
                     m_diffusion_func(x, t) * dt_alpha * noise[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }
};

/**
 * @brief Subordinated Langevin equation implementation
 *
 * This class implements the subordinated Langevin equation:
 * dx(t) = f(x(t), t) dS(t) + g(x(t), t) dB(S(t)), x(0) = x0
 *
 * where S(t) is the α-stable subordinator.
 */
export template <typename DriftFunc, typename DiffusionFunc>
class SubordinatedLangevin : public ContinuousProcess {
  static_assert(std::is_invocable_r_v<double, DriftFunc, double, double>,
                "DriftFunc must be callable with (double, double) -> double");
  static_assert(
      std::is_invocable_r_v<double, DiffusionFunc, double, double>,
      "DiffusionFunc must be callable with (double, double) -> double");

private:
  DriftFunc m_drift_func;         ///< Drift function f(x, t)
  DiffusionFunc m_diffusion_func; ///< Diffusion function g(x, t)
  double m_start_position = 0.0;  ///< Initial position
  double m_alpha = 0.5;           ///< Stability index for subordinator

public:
  /**
   * @brief Constructs a subordinated Langevin equation
   * @param drift_func The drift function f(x, t)
   * @param diffusion_func The diffusion function g(x, t)
   * @param start_position Initial position
   * @param alpha Stability index for subordinator (0 < α < 1)
   */
  SubordinatedLangevin(DriftFunc drift_func, DiffusionFunc diffusion_func,
                       double start_position = 0.0, double alpha = 0.5)
      : m_drift_func(std::move(drift_func)),
        m_diffusion_func(std::move(diffusion_func)),
        m_start_position(start_position), m_alpha(alpha) {
    if (alpha <= 0.0 || alpha >= 1.0) {
      throw std::invalid_argument("Alpha must be in range (0, 1)");
    }
  }

  /**
   * @brief Gets the initial position
   * @return The initial position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Gets the stability index
   * @return The stability index
   */
  [[nodiscard]] auto get_alpha() const -> double { return m_alpha; }

  /**
   * @brief Simulates a trajectory of the subordinated Langevin equation
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
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

    // Generate subordinator increments (positive stable)
    auto subordinator_result = rand_stable(num_steps, m_alpha, 1.0, 1.0, 0.0);
    if (!subordinator_result.has_value()) {
      return Err(subordinator_result.error());
    }
    auto subordinator_increments = subordinator_result.value();

    // Generate Brownian increments
    auto brownian_result = randn(num_steps, 0.0, 1.0);
    if (!brownian_result.has_value()) {
      return Err(brownian_result.error());
    }
    auto brownian_increments = brownian_result.value();

    // Simulate trajectory
    for (size_t i = 1; i <= num_steps; ++i) {
      double t = (i - 1) * time_step;
      double x = positions[i - 1];

      times[i] = i * time_step;

      // Subordinated time increment
      double ds = subordinator_increments[i - 1] * time_step;

      positions[i] =
          x + m_drift_func(x, t) * ds +
          m_diffusion_func(x, t) * std::sqrt(ds) * brownian_increments[i - 1];
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }
};

/**
 * @brief Helper function to create a Langevin equation
 * @param drift_func The drift function
 * @param diffusion_func The diffusion function
 * @param start_position Initial position
 * @return Langevin equation instance
 */
export template <typename DriftFunc, typename DiffusionFunc>
auto make_langevin(DriftFunc &&drift_func, DiffusionFunc &&diffusion_func,
                   double start_position = 0.0) {
  return Langevin<std::decay_t<DriftFunc>, std::decay_t<DiffusionFunc>>(
      std::forward<DriftFunc>(drift_func),
      std::forward<DiffusionFunc>(diffusion_func), start_position);
}

/**
 * @brief Helper function to create a generalized Langevin equation
 * @param drift_func The drift function
 * @param diffusion_func The diffusion function
 * @param start_position Initial position
 * @param alpha Stability index
 * @return Generalized Langevin equation instance
 */
export template <typename DriftFunc, typename DiffusionFunc>
auto make_generalized_langevin(DriftFunc &&drift_func,
                               DiffusionFunc &&diffusion_func,
                               double start_position = 0.0,
                               double alpha = 1.5) {
  return GeneralizedLangevin<std::decay_t<DriftFunc>,
                             std::decay_t<DiffusionFunc>>(
      std::forward<DriftFunc>(drift_func),
      std::forward<DiffusionFunc>(diffusion_func), start_position, alpha);
}

/**
 * @brief Helper function to create a subordinated Langevin equation
 * @param drift_func The drift function
 * @param diffusion_func The diffusion function
 * @param start_position Initial position
 * @param alpha Stability index for subordinator
 * @return Subordinated Langevin equation instance
 */
export template <typename DriftFunc, typename DiffusionFunc>
auto make_subordinated_langevin(DriftFunc &&drift_func,
                                DiffusionFunc &&diffusion_func,
                                double start_position = 0.0,
                                double alpha = 0.5) {
  return SubordinatedLangevin<std::decay_t<DriftFunc>,
                              std::decay_t<DiffusionFunc>>(
      std::forward<DriftFunc>(drift_func),
      std::forward<DiffusionFunc>(diffusion_func), start_position, alpha);
}