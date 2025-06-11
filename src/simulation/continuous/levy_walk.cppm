module;

#include <cmath>
#include <random>
#include <vector>

export module diffusionx.simulation.continuous.levy_walk;

import diffusionx.error;
import diffusionx.random.stable;
import diffusionx.random.exponential;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Lévy walk implementation
 *
 * A Lévy walk is a random walk model where the walker moves with a constant
 * velocity between turning points. At each turning point, the walker randomly
 * chooses a new direction and a new flight time τ from a probability
 * distribution ψ(τ) ~ τ^(-1-α) with 0 < α < 1. The flight length is
 * proportional to the flight time: l = vτ, where v is the constant velocity.
 */
export class LevyWalk : public ContinuousProcess {
  double m_alpha = 0.5;          ///< Waiting time distribution exponent
  double m_velocity = 1.0;       ///< Constant velocity
  double m_start_position = 0.0; ///< Starting position

public:
  /**
   * @brief Default constructor creating a standard Lévy walk
   *
   * Creates a Lévy walk with α = 0.5, velocity = 1.0, starting at position 0.
   */
  LevyWalk() = default;

  /**
   * @brief Constructs a Lévy walk with specified parameters
   * @param alpha Waiting time distribution exponent (0 < α < 1)
   * @param velocity Constant velocity (must be positive)
   * @param start_position Starting position
   * @throws std::invalid_argument if parameters are invalid
   */
  LevyWalk(double alpha, double velocity, double start_position = 0.0)
      : m_alpha(alpha), m_velocity(velocity), m_start_position(start_position) {
    if (m_alpha <= 0.0 || m_alpha >= 1.0) {
      throw std::invalid_argument("Alpha must be in range (0, 1)");
    }
    if (m_velocity <= 0.0) {
      throw std::invalid_argument("Velocity must be positive");
    }
  }

  /**
   * @brief Gets the waiting time distribution exponent
   * @return The exponent α
   */
  [[nodiscard]] auto get_alpha() const -> double { return m_alpha; }

  /**
   * @brief Gets the velocity
   * @return The velocity
   */
  [[nodiscard]] auto get_velocity() const -> double { return m_velocity; }

  /**
   * @brief Gets the starting position
   * @return The starting position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Simulates a trajectory of the Lévy walk
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * The algorithm:
   * 1. Generate waiting times from power-law distribution
   * 2. Generate random directions (±1)
   * 3. Move with constant velocity during each flight
   * 4. Interpolate to regular time grid
   */
  Result<vec_pair> simulate(double duration, double time_step = 0.01) override {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }
    if (time_step <= 0) {
      return Err(Error::InvalidArgument("Time step must be positive"));
    }

    // Generate event times and positions
    vector<double> event_times;
    vector<double> event_positions;

    event_times.push_back(0.0);
    event_positions.push_back(m_start_position);

    double current_time = 0.0;
    double current_position = m_start_position;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);
    std::uniform_int_distribution<int> direction_dist(0, 1);

    // Generate events until we exceed the duration
    while (current_time < duration) {
      // Generate waiting time from power-law distribution
      // Using inverse transform sampling: τ = (1 - U)^(-1/α) - 1
      double u = uniform_dist(gen);
      double waiting_time = std::pow(1.0 - u, -1.0 / m_alpha) - 1.0;

      // Limit waiting time to avoid extremely long flights
      waiting_time = std::min(waiting_time, duration - current_time);

      // Generate random direction
      int direction = direction_dist(gen) * 2 - 1; // -1 or +1

      // Update time and position
      current_time += waiting_time;
      current_position += direction * m_velocity * waiting_time;

      event_times.push_back(current_time);
      event_positions.push_back(current_position);
    }

    // Interpolate to regular time grid
    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    for (size_t i = 0; i <= num_steps; ++i) {
      times[i] = i * time_step;
      positions[i] =
          interpolate_position(times[i], event_times, event_positions);
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

private:
  /**
   * @brief Interpolates position at given time using piecewise linear
   * interpolation
   * @param t Target time
   * @param event_times Vector of event times
   * @param event_positions Vector of event positions
   * @return Interpolated position
   */
  double interpolate_position(double t, const vector<double> &event_times,
                              const vector<double> &event_positions) const {
    if (t <= event_times[0]) {
      return event_positions[0];
    }
    if (t >= event_times.back()) {
      return event_positions.back();
    }

    // Find the interval containing t
    for (size_t i = 1; i < event_times.size(); ++i) {
      if (t <= event_times[i]) {
        // Linear interpolation between event_times[i-1] and event_times[i]
        double t1 = event_times[i - 1];
        double t2 = event_times[i];
        double x1 = event_positions[i - 1];
        double x2 = event_positions[i];

        return x1 + (x2 - x1) * (t - t1) / (t2 - t1);
      }
    }

    return event_positions.back();
  }
};

/**
 * @brief Coupled Continuous Time Random Walk (CTRW) implementation
 *
 * This is a generalization of Lévy walk where both waiting times and jump sizes
 * can follow heavy-tailed distributions.
 */
export class CoupledCTRW : public ContinuousProcess {
  double m_alpha = 0.5;          ///< Waiting time distribution exponent
  double m_beta = 1.5;           ///< Jump size distribution exponent
  double m_start_position = 0.0; ///< Starting position

public:
  /**
   * @brief Default constructor creating a standard coupled CTRW
   */
  CoupledCTRW() = default;

  /**
   * @brief Constructs a coupled CTRW with specified parameters
   * @param alpha Waiting time distribution exponent (0 < α < 1)
   * @param beta Jump size distribution exponent (0 < β ≤ 2)
   * @param start_position Starting position
   * @throws std::invalid_argument if parameters are invalid
   */
  CoupledCTRW(double alpha, double beta, double start_position = 0.0)
      : m_alpha(alpha), m_beta(beta), m_start_position(start_position) {
    if (m_alpha <= 0.0 || m_alpha >= 1.0) {
      throw std::invalid_argument("Alpha must be in range (0, 1)");
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
   * @brief Simulates a trajectory of the coupled CTRW
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

    // Generate event times and positions
    vector<double> event_times;
    vector<double> event_positions;

    event_times.push_back(0.0);
    event_positions.push_back(m_start_position);

    double current_time = 0.0;
    double current_position = m_start_position;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);

    // Generate events until we exceed the duration
    while (current_time < duration) {
      // Generate waiting time from power-law distribution
      double u1 = uniform_dist(gen);
      double waiting_time = std::pow(1.0 - u1, -1.0 / m_alpha) - 1.0;
      waiting_time = std::min(waiting_time, duration - current_time);

      // Generate jump size from stable distribution
      auto jump_result = rand_stable(1, m_beta, 0.0, 1.0, 0.0);
      if (!jump_result.has_value()) {
        return Err(jump_result.error());
      }
      auto jumps = jump_result.value();
      double jump_size = jumps[0];

      // Update time and position
      current_time += waiting_time;
      current_position += jump_size;

      event_times.push_back(current_time);
      event_positions.push_back(current_position);
    }

    // Interpolate to regular time grid (piecewise constant)
    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    for (size_t i = 0; i <= num_steps; ++i) {
      times[i] = i * time_step;
      positions[i] =
          interpolate_position_constant(times[i], event_times, event_positions);
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

private:
  /**
   * @brief Interpolates position using piecewise constant interpolation
   * @param t Target time
   * @param event_times Vector of event times
   * @param event_positions Vector of event positions
   * @return Interpolated position
   */
  double
  interpolate_position_constant(double t, const vector<double> &event_times,
                                const vector<double> &event_positions) const {
    if (t <= event_times[0]) {
      return event_positions[0];
    }

    // Find the last event time before or at t
    for (size_t i = 1; i < event_times.size(); ++i) {
      if (t < event_times[i]) {
        return event_positions[i - 1];
      }
    }

    return event_positions.back();
  }
};