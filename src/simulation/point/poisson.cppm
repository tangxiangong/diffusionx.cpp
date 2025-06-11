module;

#include <cmath>
#include <random>
#include <vector>

export module diffusionx.simulation.point.poisson;

import diffusionx.error;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.basic.abstract;

using std::vector;

/**
 * @brief Poisson process implementation
 *
 * A Poisson process is a point process where events occur independently
 * at a constant average rate λ. The inter-arrival times follow an
 * exponential distribution with parameter λ.
 */
export class PoissonProcess {
  double m_rate = 1.0; ///< Event rate λ

public:
  /**
   * @brief Default constructor creating a unit rate Poisson process
   */
  PoissonProcess() = default;

  /**
   * @brief Constructs a Poisson process with specified rate
   * @param rate Event rate λ (must be positive)
   * @throws std::invalid_argument if rate is not positive
   */
  explicit PoissonProcess(double rate) : m_rate(rate) {
    if (m_rate <= 0.0) {
      throw std::invalid_argument("Rate must be positive");
    }
  }

  /**
   * @brief Gets the event rate
   * @return The event rate λ
   */
  [[nodiscard]] auto get_rate() const -> double { return m_rate; }

  /**
   * @brief Simulates the Poisson process
   * @param duration The total simulation time
   * @return Result containing event times, or an Error
   */
  Result<vector<double>> simulate(double duration) {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }

    vector<double> event_times;
    double current_time = 0.0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<double> exp_dist(m_rate);

    while (current_time < duration) {
      double inter_arrival_time = exp_dist(gen);
      current_time += inter_arrival_time;

      if (current_time < duration) {
        event_times.push_back(current_time);
      }
    }

    return Ok(std::move(event_times));
  }

  /**
   * @brief Simulates the Poisson process and returns counting process
   * trajectory
   * @param duration The total simulation time
   * @return Result containing time and count vectors, or an Error
   */
  Result<vec_pair> simulate_trajectory(double duration) {
    auto events_result = simulate(duration);
    if (!events_result.has_value()) {
      return Err(events_result.error());
    }
    auto event_times = events_result.value();

    // Create counting process trajectory
    vector<double> times;
    vector<double> counts;

    times.push_back(0.0);
    counts.push_back(0.0);

    for (size_t i = 0; i < event_times.size(); ++i) {
      times.push_back(event_times[i]);
      counts.push_back(static_cast<double>(i + 1));
    }

    times.push_back(duration);
    counts.push_back(static_cast<double>(event_times.size()));

    return Ok(std::make_pair(std::move(times), std::move(counts)));
  }

  /**
   * @brief Simulates the Poisson process with a given number of steps
   * @param num_steps The number of steps to simulate
   * @return Result containing time and count vectors, or an Error
   */
  Result<vec_pair> simulate_with_steps(size_t num_steps) {
    if (num_steps == 0) {
      return Err(Error::InvalidArgument("Number of steps must be positive"));
    }

    vector<double> times(num_steps + 1);
    vector<double> counts(num_steps + 1);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<double> exp_dist(m_rate);

    times[0] = 0.0;
    counts[0] = 0.0;

    for (size_t i = 1; i <= num_steps; ++i) {
      double inter_arrival_time = exp_dist(gen);
      times[i] = times[i - 1] + inter_arrival_time;
      counts[i] = static_cast<double>(i);
    }

    return Ok(std::make_pair(std::move(times), std::move(counts)));
  }

  /**
   * @brief Computes the theoretical mean number of events in time interval
   * @param t Time interval length
   * @return The theoretical mean E[N(t)] = λt
   */
  [[nodiscard]] auto theoretical_mean(double t) const -> double {
    return m_rate * t;
  }

  /**
   * @brief Computes the theoretical variance of number of events
   * @param t Time interval length
   * @return The theoretical variance Var[N(t)] = λt
   */
  [[nodiscard]] auto theoretical_variance(double t) const -> double {
    return m_rate * t;
  }
};