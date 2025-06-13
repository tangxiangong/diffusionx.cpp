/**
 * @file functional.cppm
 * @brief Functional distribution simulation module
 *
 * This module provides the basic infrastructure for simulating functional
 * distribution.
 *
 * - First passage time
 * - Occupation time
 *
 */

module;

#include <format>

export module diffusionx.simulation.basic.functional;

import diffusionx.error;
import diffusionx.simulation.basic.utils;

/**
 * @brief First passage time simulation class
 *
 * This class represents the first passage time of a stochastic process through
 * a given domain. The first passage time is defined as the first time the
 * process exits the interval (a, b).
 *
 * @tparam T The type of the stochastic process
 *
 * Mathematical definition:
 * τ = inf{t ≥ 0 : X(t) ∉ (a, b)}
 *
 * where X(t) is the stochastic process and (a, b) is the domain interval.
 *
 * @example
 * ```cpp
 * BrownianMotion process;
 * FirstPassageTime fpt(process, {-1.0, 1.0});
 * ```
 */
export template <typename T> class FirstPassageTime {
  T &m_process;         ///< Reference to the stochastic process
  double_pair m_domain; ///< The domain interval (a, b)

public:
  /**
   * @brief Default constructor
   *
   * Creates an uninitialized FirstPassageTime object.
   * This constructor should only be used when the object will be assigned
   * later.
   */
  FirstPassageTime() = default;

  /**
   * @brief Construct a FirstPassageTime object
   *
   * @param process Reference to the stochastic process
   * @param domain The domain interval (a, b) where a < b
   *
   * @throws std::invalid_argument if the domain is not a valid interval (a >=
   * b)
   *
   * @pre The domain must satisfy a < b
   * @post The FirstPassageTime object is properly initialized
   *
   * @example
   * ```cpp
   * BrownianMotion bm;
   * FirstPassageTime fpt(bm, {-2.0, 2.0});  // Valid interval
   * ```
   */
  FirstPassageTime(T &process, double_pair domain)
      : m_process{process}, m_domain{domain} {
    auto [a, b] = domain;
    if (a >= b) {
      throw std::invalid_argument(std::format(
          "The domain (a, b) must be a valid interval, but got ({}, {})", a,
          b));
    }
  }

  /**
   * @brief Get the associated stochastic process
   *
   * @return T A copy of the stochastic process
   *
   * @note This returns a copy of the process, not a reference
   */
  T get_process() const { return m_process; }

  /**
   * @brief Get the domain interval
   *
   * @return double_pair The domain interval (a, b)
   *
   * @post The returned pair represents the interval bounds where first < second
   */
  [[nodiscard]] double_pair get_domain() const { return m_domain; }

  Result<Option<double>> simulate(double max_duration,
                                  double time_step = 0.01) {
    return Err(Error::NotImplemented(
        "first passage time simulation is not implemented"));
  }
};

/**
 * @brief Occupation time simulation class
 *
 * This class represents the occupation time of a stochastic process within a
 * given domain over a specified duration. The occupation time measures how long
 * the process spends within the interval (a, b) during the time period [0, T].
 *
 * @tparam T The type of the stochastic process
 *
 * Mathematical definition:
 * L_T = ∫₀ᵀ 𝟙_{(a,b)}(X(t)) dt
 *
 * where X(t) is the stochastic process, (a, b) is the domain interval,
 * T is the duration, and 𝟙_{(a,b)} is the indicator function.
 *
 * @example
 * ```cpp
 * BrownianMotion process;
 * OccupationTime ot(process, {-1.0, 1.0}, 10.0);
 * ```
 */
export template <typename T> class OccupationTime {
  T &m_process;         ///< Reference to the stochastic process
  double_pair m_domain; ///< The domain interval (a, b)
  double m_duration;    ///< The time duration T

public:
  /**
   * @brief Default constructor
   *
   * Creates an uninitialized OccupationTime object.
   * This constructor should only be used when the object will be assigned
   * later.
   */
  OccupationTime() = default;

  /**
   * @brief Construct an OccupationTime object
   *
   * @param process Reference to the stochastic process
   * @param domain The domain interval (a, b) where a < b
   * @param duration The time duration T > 0
   *
   * @throws std::invalid_argument if the domain is not a valid interval (a >=
   * b)
   * @throws std::invalid_argument if the duration is not positive (duration <
   * 0)
   *
   * @pre The domain must satisfy a < b
   * @pre The duration must be positive
   * @post The OccupationTime object is properly initialized
   *
   * @example
   * ```cpp
   * BrownianMotion bm;
   * OccupationTime ot(bm, {-1.5, 1.5}, 5.0);  // Valid parameters
   * ```
   */
  OccupationTime(T &process, double_pair domain, double duration)
      : m_process{process}, m_domain{domain}, m_duration{duration} {
    auto [a, b] = domain;
    if (a >= b) {
      throw std::invalid_argument(std::format(
          "The domain (a, b) must be a valid interval, but got ({}, {})", a,
          b));
    }
    if (duration < 0) {
      throw std::invalid_argument(
          std::format("The `duration` must be positive, but got {}", duration));
    }
  }

  /**
   * @brief Get the time duration
   *
   * @return double The time duration T
   *
   * @post The returned value is positive
   */
  [[nodiscard]] double get_duration() const { return m_duration; }

  /**
   * @brief Get the domain interval
   *
   * @return double_pair The domain interval (a, b)
   *
   * @post The returned pair represents the interval bounds where first < second
   */
  [[nodiscard]] double_pair get_domain() const { return m_domain; }

  /**
   * @brief Get the associated stochastic process
   *
   * @return T A copy of the stochastic process
   *
   * @note This returns a copy of the process, not a reference
   */
  T get_process() const { return m_process; }
};
