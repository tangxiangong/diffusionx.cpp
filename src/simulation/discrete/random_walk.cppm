module;

#include <random>
#include <vector>

export module diffusionx.simulation.discrete.random_walk;

import diffusionx.error;
import diffusionx.random.uniform;
import diffusionx.random.stable;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Simple random walk implementation
 *
 * This class implements a simple symmetric random walk on the integers,
 * where at each step the walker moves +1 or -1 with equal probability.
 *
 * Mathematical definition:
 * X(n) = X(0) + Σ(i=1 to n) ξ_i
 *
 * where ξ_i are i.i.d. random variables taking values ±1 with probability 1/2
 * each.
 */
export class SimpleRandomWalk : public DiscreteProcess {
  double m_start_position = 0.0; ///< Initial position
  double m_step_size = 1.0;      ///< Size of each step

public:
  /**
   * @brief Default constructor creating standard random walk
   */
  SimpleRandomWalk() = default;

  /**
   * @brief Constructs random walk with specified parameters
   * @param start_position Initial position
   * @param step_size Size of each step (must be positive)
   * @throws std::invalid_argument if step_size is not positive
   */
  SimpleRandomWalk(double start_position, double step_size = 1.0)
      : m_start_position(start_position), m_step_size(step_size) {
    if (m_step_size <= 0.0) {
      throw std::invalid_argument("Step size must be positive");
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
   * @brief Gets the step size
   * @return The step size
   */
  [[nodiscard]] auto get_step_size() const -> double { return m_step_size; }

  /**
   * @brief Simulates a trajectory of the random walk
   * @param num_steps The number of steps to simulate
   * @return Result containing step numbers and position vectors, or an Error
   */
  Result<discrete_pair> simulate(size_t num_steps) override {
    if (num_steps == 0) {
      return Ok(
          std::make_pair(vector<size_t>{0}, vector<double>{m_start_position}));
    }

    vector<size_t> steps(num_steps + 1);
    vector<double> positions(num_steps + 1);

    // Initialize
    steps[0] = 0;
    positions[0] = m_start_position;

    // Generate random steps (+1 or -1)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 1);

    // Simulate trajectory
    for (size_t i = 1; i <= num_steps; ++i) {
      steps[i] = i;
      int direction = (dist(gen) == 0) ? -1 : 1;
      positions[i] = positions[i - 1] + direction * m_step_size;
    }

    return Ok(std::make_pair(std::move(steps), std::move(positions)));
  }

  /**
   * @brief Computes the theoretical variance at step n
   * @param n Step number
   * @return The theoretical variance Var[X(n)]
   */
  [[nodiscard]] auto theoretical_variance(size_t n) const -> double {
    return n * m_step_size * m_step_size;
  }
};

/**
 * @brief Biased random walk implementation
 *
 * This class implements a biased random walk where the probability of moving
 * in the positive direction is p and in the negative direction is (1-p).
 */
export class BiasedRandomWalk : public DiscreteProcess {
  double m_start_position = 0.0; ///< Initial position
  double m_step_size = 1.0;      ///< Size of each step
  double m_probability = 0.5; ///< Probability of moving in positive direction

public:
  /**
   * @brief Default constructor creating symmetric random walk
   */
  BiasedRandomWalk() = default;

  /**
   * @brief Constructs biased random walk with specified parameters
   * @param start_position Initial position
   * @param probability Probability of moving in positive direction (must be in
   * [0, 1])
   * @param step_size Size of each step (must be positive)
   * @throws std::invalid_argument if parameters are invalid
   */
  BiasedRandomWalk(double start_position, double probability,
                   double step_size = 1.0)
      : m_start_position(start_position), m_step_size(step_size),
        m_probability(probability) {
    if (m_probability < 0.0 || m_probability > 1.0) {
      throw std::invalid_argument("Probability must be in [0, 1]");
    }
    if (m_step_size <= 0.0) {
      throw std::invalid_argument("Step size must be positive");
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
   * @brief Gets the step size
   * @return The step size
   */
  [[nodiscard]] auto get_step_size() const -> double { return m_step_size; }

  /**
   * @brief Gets the probability of positive step
   * @return The probability of moving in positive direction
   */
  [[nodiscard]] auto get_probability() const -> double { return m_probability; }

  /**
   * @brief Simulates a trajectory of the biased random walk
   * @param num_steps The number of steps to simulate
   * @return Result containing step numbers and position vectors, or an Error
   */
  Result<discrete_pair> simulate(size_t num_steps) override {
    if (num_steps == 0) {
      return Ok(
          std::make_pair(vector<size_t>{0}, vector<double>{m_start_position}));
    }

    vector<size_t> steps(num_steps + 1);
    vector<double> positions(num_steps + 1);

    // Initialize
    steps[0] = 0;
    positions[0] = m_start_position;

    // Generate random steps
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // Simulate trajectory
    for (size_t i = 1; i <= num_steps; ++i) {
      steps[i] = i;
      int direction = (dist(gen) < m_probability) ? 1 : -1;
      positions[i] = positions[i - 1] + direction * m_step_size;
    }

    return Ok(std::make_pair(std::move(steps), std::move(positions)));
  }

  /**
   * @brief Computes the theoretical mean at step n
   * @param n Step number
   * @return The theoretical mean E[X(n)]
   */
  [[nodiscard]] auto theoretical_mean(size_t n) const -> double {
    return m_start_position + n * m_step_size * (2.0 * m_probability - 1.0);
  }

  /**
   * @brief Computes the theoretical variance at step n
   * @param n Step number
   * @return The theoretical variance Var[X(n)]
   */
  [[nodiscard]] auto theoretical_variance(size_t n) const -> double {
    return 4.0 * n * m_probability * (1.0 - m_probability) * m_step_size *
           m_step_size;
  }
};

/**
 * @brief Lévy random walk implementation
 *
 * This class implements a random walk with step sizes drawn from a stable
 * distribution, generalizing the simple random walk to heavy-tailed step
 * distributions.
 */
export class LevyRandomWalk : public DiscreteProcess {
  double m_alpha = 2.0;          ///< Stability parameter α ∈ (0, 2]
  double m_beta = 0.0;           ///< Skewness parameter β ∈ [-1, 1]
  double m_sigma = 1.0;          ///< Scale parameter σ > 0
  double m_start_position = 0.0; ///< Initial position

public:
  /**
   * @brief Default constructor creating Gaussian random walk (α = 2)
   */
  LevyRandomWalk() = default;

  /**
   * @brief Constructs Lévy random walk with specified parameters
   * @param alpha Stability parameter α (must be in (0, 2])
   * @param beta Skewness parameter β (must be in [-1, 1])
   * @param sigma Scale parameter σ (must be positive)
   * @param start_position Initial position
   * @throws std::invalid_argument if parameters are invalid
   */
  LevyRandomWalk(double alpha, double beta, double sigma,
                 double start_position = 0.0)
      : m_alpha(alpha), m_beta(beta), m_sigma(sigma),
        m_start_position(start_position) {
    if (m_alpha <= 0.0 || m_alpha > 2.0) {
      throw std::invalid_argument(
          "Stability parameter alpha must be in (0, 2]");
    }
    if (m_beta < -1.0 || m_beta > 1.0) {
      throw std::invalid_argument("Skewness parameter beta must be in [-1, 1]");
    }
    if (m_sigma <= 0.0) {
      throw std::invalid_argument("Scale parameter sigma must be positive");
    }
  }

  /**
   * @brief Gets the stability parameter
   * @return The stability parameter α
   */
  [[nodiscard]] auto get_alpha() const -> double { return m_alpha; }

  /**
   * @brief Gets the skewness parameter
   * @return The skewness parameter β
   */
  [[nodiscard]] auto get_beta() const -> double { return m_beta; }

  /**
   * @brief Gets the scale parameter
   * @return The scale parameter σ
   */
  [[nodiscard]] auto get_sigma() const -> double { return m_sigma; }

  /**
   * @brief Gets the initial position
   * @return The initial position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Simulates a trajectory of the Lévy random walk
   * @param num_steps The number of steps to simulate
   * @return Result containing step numbers and position vectors, or an Error
   */
  Result<discrete_pair> simulate(size_t num_steps) override {
    if (num_steps == 0) {
      return Ok(
          std::make_pair(vector<size_t>{0}, vector<double>{m_start_position}));
    }

    vector<size_t> steps(num_steps + 1);
    vector<double> positions(num_steps + 1);

    // Initialize
    steps[0] = 0;
    positions[0] = m_start_position;

    // Generate stable increments
    auto increments_result =
        rand_stable(num_steps, m_alpha, m_beta, m_sigma, 0.0);
    if (!increments_result.has_value()) {
      return Err(increments_result.error());
    }
    auto increments = increments_result.value();

    // Simulate trajectory
    for (size_t i = 1; i <= num_steps; ++i) {
      steps[i] = i;
      positions[i] = positions[i - 1] + increments[i - 1];
    }

    return Ok(std::make_pair(std::move(steps), std::move(positions)));
  }

  /**
   * @brief Computes the theoretical mean at step n (when it exists)
   * @param n Step number
   * @return The theoretical mean E[X(n)] if α > 1, otherwise NaN
   */
  [[nodiscard]] auto theoretical_mean(size_t n) const -> double {
    if (m_alpha > 1.0) {
      return m_start_position; // Mean of stable distribution is 0 for our
                               // parameterization
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

  /**
   * @brief Checks if the process has finite variance
   * @return True if α = 2, false otherwise
   */
  [[nodiscard]] auto has_finite_variance() const -> bool {
    return std::abs(m_alpha - 2.0) < 1e-10;
  }
};