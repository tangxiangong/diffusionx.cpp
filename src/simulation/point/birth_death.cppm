module;

#include <cmath>
#include <random>
#include <vector>

export module diffusionx.simulation.point.birth_death;

import diffusionx.error;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.basic.abstract;

using std::vector;

/**
 * @brief Birth-death process implementation
 *
 * A birth-death process is a continuous-time Markov chain where
 * the state can only increase by 1 (birth) or decrease by 1 (death).
 * The birth rate is λ and the death rate is μ.
 *
 * Mathematical formulation:
 * - Birth rate: λ(n) = λ * n (linear birth rate)
 * - Death rate: μ(n) = μ * n (linear death rate)
 * - Total rate: λ(n) + μ(n) = (λ + μ) * n
 */
export class BirthDeathProcess {
  double m_birth_rate = 1.0; ///< Birth rate λ
  double m_death_rate = 0.5; ///< Death rate μ
  int m_initial_state = 1;   ///< Initial population

public:
  /**
   * @brief Default constructor
   */
  BirthDeathProcess() = default;

  /**
   * @brief Constructs a birth-death process with specified parameters
   * @param birth_rate Birth rate λ (must be positive)
   * @param death_rate Death rate μ (must be non-negative)
   * @param initial_state Initial population (must be non-negative)
   * @throws std::invalid_argument if parameters are invalid
   */
  BirthDeathProcess(double birth_rate, double death_rate, int initial_state = 1)
      : m_birth_rate(birth_rate), m_death_rate(death_rate),
        m_initial_state(initial_state) {
    if (m_birth_rate <= 0.0) {
      throw std::invalid_argument("Birth rate must be positive");
    }
    if (m_death_rate < 0.0) {
      throw std::invalid_argument("Death rate must be non-negative");
    }
    if (m_initial_state < 0) {
      throw std::invalid_argument("Initial state must be non-negative");
    }
  }

  /**
   * @brief Gets the birth rate
   * @return The birth rate λ
   */
  [[nodiscard]] auto get_birth_rate() const -> double { return m_birth_rate; }

  /**
   * @brief Gets the death rate
   * @return The death rate μ
   */
  [[nodiscard]] auto get_death_rate() const -> double { return m_death_rate; }

  /**
   * @brief Gets the initial state
   * @return The initial population
   */
  [[nodiscard]] auto get_initial_state() const -> int {
    return m_initial_state;
  }

  /**
   * @brief Simulates the birth-death process with a given number of steps
   * @param num_steps The number of steps to simulate
   * @return Result containing time and state vectors, or an Error
   */
  Result<vec_pair> simulate_with_steps(size_t num_steps) {
    if (num_steps == 0) {
      return Err(Error::InvalidArgument("Number of steps must be positive"));
    }

    vector<double> times;
    vector<double> states;

    times.push_back(0.0);
    states.push_back(static_cast<double>(m_initial_state));

    double current_time = 0.0;
    int current_state = m_initial_state;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);

    for (size_t step = 0; step < num_steps; ++step) {
      if (current_state == 0) {
        // Population extinct, no more transitions possible
        break;
      }

      // Calculate total rate
      double total_rate = (m_birth_rate + m_death_rate) * current_state;

      // Generate time to next event
      std::exponential_distribution<double> exp_dist(total_rate);
      double time_to_next = exp_dist(gen);
      current_time += time_to_next;

      // Determine type of event
      double u = uniform_dist(gen);
      double birth_prob = m_birth_rate / (m_birth_rate + m_death_rate);

      if (u < birth_prob) {
        // Birth event
        current_state++;
      } else {
        // Death event
        current_state--;
        if (current_state < 0) {
          current_state = 0; // Prevent negative population
        }
      }

      times.push_back(current_time);
      states.push_back(static_cast<double>(current_state));
    }

    return Ok(std::make_pair(std::move(times), std::move(states)));
  }

  /**
   * @brief Simulates the birth-death process trajectory
   * @param duration The total simulation time
   * @return Result containing time and state vectors, or an Error
   */
  Result<vec_pair> simulate_with_duration(double duration) {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }

    vector<double> times;
    vector<double> states;

    times.push_back(0.0);
    states.push_back(static_cast<double>(m_initial_state));

    double current_time = 0.0;
    int current_state = m_initial_state;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);

    while (current_time < duration) {
      if (current_state == 0) {
        // Population extinct, no more transitions possible
        break;
      }

      // Calculate total rate
      double total_rate = (m_birth_rate + m_death_rate) * current_state;

      // Generate time to next event
      std::exponential_distribution<double> exp_dist(total_rate);
      double time_to_next = exp_dist(gen);
      current_time += time_to_next;

      if (current_time >= duration) {
        break;
      }

      // Determine type of event
      double u = uniform_dist(gen);
      double birth_prob = m_birth_rate / (m_birth_rate + m_death_rate);

      if (u < birth_prob) {
        // Birth event
        current_state++;
      } else {
        // Death event
        current_state--;
        if (current_state < 0) {
          current_state = 0; // Prevent negative population
        }
      }

      times.push_back(current_time);
      states.push_back(static_cast<double>(current_state));
    }

    // Add final point at duration
    times.push_back(duration);
    states.push_back(static_cast<double>(current_state));

    return Ok(std::make_pair(std::move(times), std::move(states)));
  }

  /**
   * @brief Simulates the birth-death process and interpolates to regular time
   * grid
   * @param duration The total simulation time
   * @param time_step The time step for interpolation
   * @return Result containing time and state vectors, or an Error
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

    auto [event_times, event_states] = trajectory_result.value();

    // Interpolate to regular time grid using piecewise constant interpolation
    size_t num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> states(num_steps + 1);

    for (size_t i = 0; i <= num_steps; ++i) {
      times[i] = i * time_step;

      // Find the state at this time using piecewise constant interpolation
      auto it =
          std::upper_bound(event_times.begin(), event_times.end(), times[i]);
      if (it == event_times.begin()) {
        states[i] = static_cast<double>(m_initial_state);
      } else {
        size_t index = std::distance(event_times.begin(), it) - 1;
        states[i] = event_states[index];
      }
    }

    return Ok(std::make_pair(std::move(times), std::move(states)));
  }

  /**
   * @brief Computes the theoretical mean population at time t
   * @param t Time point
   * @return The theoretical mean E[N(t)]
   */
  [[nodiscard]] auto theoretical_mean(double t) const -> double {
    if (m_birth_rate == m_death_rate) {
      return static_cast<double>(m_initial_state);
    } else {
      double r = m_birth_rate - m_death_rate;
      return static_cast<double>(m_initial_state) * std::exp(r * t);
    }
  }

  /**
   * @brief Computes the theoretical variance of population at time t
   * @param t Time point
   * @return The theoretical variance Var[N(t)]
   */
  [[nodiscard]] auto theoretical_variance(double t) const -> double {
    if (m_birth_rate == m_death_rate) {
      return static_cast<double>(m_initial_state) *
             (m_birth_rate + m_death_rate) * t;
    } else {
      double r = m_birth_rate - m_death_rate;
      double exp_rt = std::exp(r * t);
      return static_cast<double>(m_initial_state) *
             (m_birth_rate + m_death_rate) / r * exp_rt * (exp_rt - 1.0);
    }
  }
};