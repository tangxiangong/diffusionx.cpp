/**
 * @file discrete.cppm
 * @brief Discrete stochastic process simulation
 * 
 * This module provides the core infrastructure for simulating discrete stochastic processes.
 */

module;

#include <concepts>
#include <vector>
#include <cmath>
#include <string>

export module diffusionx.simulation.basic.discrete;

import diffusionx.error;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.basic.moment;

using std::vector;

/**
 * @brief Type alias for discrete trajectory data (step numbers, positions)
 */
export using discrete_pair = std::pair<vector<size_t>, vector<double>>;

/**
 * @brief Abstract base class for discrete stochastic processes
 * 
 * This class provides a common interface for all discrete stochastic processes.
 * It defines virtual methods for simulation, statistical analysis, and trajectory
 * properties that derived classes must implement.
 */
export struct DiscreteProcess {
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~DiscreteProcess() = default;

    /**
     * @brief Simulates a trajectory of the discrete stochastic process
     * @param num_steps The number of steps to simulate
     * @return Result containing a pair of step numbers and position vectors, or an Error
     * 
     * This pure virtual method must be implemented by derived classes to generate
     * a single trajectory of the discrete stochastic process.
     */
    virtual Result<discrete_pair> simulate(size_t num_steps) = 0;

    /**
     * @brief Computes the mean of the process at a given step
     * @param num_steps The number of steps
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @return Result containing the mean value, or an Error
     */
    virtual Result<double> mean(size_t num_steps, size_t particles = 10000);

    /**
     * @brief Computes the mean squared displacement (MSD) at a given step
     * @param num_steps The number of steps
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @return Result containing the MSD value, or an Error
     */
    virtual Result<double> msd(size_t num_steps, size_t particles = 10000);

    /**
     * @brief Computes the raw moment of the process at a given step
     * @param num_steps The number of steps
     * @param order The order of the moment
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @return Result containing the raw moment value, or an Error
     */
    virtual Result<double> raw_moment(size_t num_steps, int order, size_t particles = 10000);

    /**
     * @brief Computes the central moment of the process at a given step
     * @param num_steps The number of steps
     * @param order The order of the moment
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @return Result containing the central moment value, or an Error
     */
    virtual Result<double> central_moment(size_t num_steps, int order, size_t particles = 10000);
};

/**
 * @brief Concept defining types that derive from DiscreteProcess
 * @tparam T The type to check
 * 
 * This concept ensures that template parameters are derived from DiscreteProcess.
 */
export template<typename T>
concept DP = std::derived_from<T, DiscreteProcess>;

/**
 * @brief Specialized template for computing moments of discrete processes
 * @tparam T The type of the discrete process (must satisfy DP concept)
 * 
 * This specialization provides concrete implementations for computing moments
 * of discrete stochastic processes.
 */
template<DP T>
struct Moment<T> {
    size_t m_num_steps; ///< The number of steps at which to compute the moment
    int m_order; ///< The order of the moment
    T &process; ///< Reference to the discrete process

    /**
     * @brief Computes the raw moment using parallel Monte Carlo simulation
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @return Result containing the raw moment value, or an Error
     */
    auto raw_moment(size_t particles = 10000) -> Result<double> {
        auto compute_func = [this](auto &proc) -> Result<double> {
            auto traj = proc.simulate(m_num_steps);
            if (!traj.has_value()) {
                return Err(traj.error());
            }
            auto [steps, positions] = traj.value();
            if (positions.empty()) {
                return Err(Error::InvalidArgument("Empty trajectory"));
            }
            return std::pow(positions.back(), m_order);
        };

        return parallel_monte_carlo(particles, process, compute_func);
    }

    /**
     * @brief Computes the central moment using parallel Monte Carlo simulation
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @return Result containing the central moment value, or an Error
     */
    auto central_moment(size_t particles = 10000) -> Result<double> {
        // First compute the mean using raw moment of order 1
        Moment<T> mean_moment(m_num_steps, 1, process);
        auto mean_result = mean_moment.raw_moment(particles);
        if (!mean_result.has_value()) {
            return Err(mean_result.error());
        }
        double mean = mean_result.value();

        auto compute_func = [this, mean](auto &proc) -> Result<double> {
            auto traj = proc.simulate(m_num_steps);
            if (!traj.has_value()) {
                return Err(traj.error());
            }
            auto [steps, positions] = traj.value();
            if (positions.empty()) {
                return Err(Error::InvalidArgument("Empty trajectory"));
            }
            return std::pow(positions.back() - mean, m_order);
        };

        return parallel_monte_carlo(particles, process, compute_func);
    }
};

Result<double> DiscreteProcess::mean(size_t num_steps, size_t particles) {
    auto moment = Moment<DiscreteProcess>(num_steps, 1, *this);
    return moment.raw_moment(particles);
}

Result<double> DiscreteProcess::msd(size_t num_steps, size_t particles) {
    auto moment = Moment<DiscreteProcess>(num_steps, 2, *this);
    return moment.central_moment(particles);
}

auto DiscreteProcess::raw_moment(size_t num_steps, int order, size_t particles) -> Result<double> {
    auto moment = Moment<DiscreteProcess>(num_steps, order, *this);
    return moment.raw_moment(particles);
}

auto DiscreteProcess::central_moment(size_t num_steps, int order, size_t particles) -> Result<double> {
    auto moment = Moment<DiscreteProcess>(num_steps, order, *this);
    return moment.central_moment(particles);
}

/**
 * @brief Structure representing a discrete trajectory with a specific number of steps
 * @tparam T The type of the discrete process (must satisfy DP concept)
 * 
 * This structure encapsulates a discrete process with a fixed number of simulation steps.
 */
export template<DP T>
struct DiscreteTrajectory {
    size_t num_steps; ///< The number of simulation steps
    T &process; ///< Reference to the discrete process

    /**
     * @brief Constructs a discrete trajectory with specified steps and process
     * @param num_steps The number of simulation steps
     * @param process Reference to the discrete process
     */
    DiscreteTrajectory(size_t num_steps, T &process)
        : num_steps(num_steps), process(process) {
    }
}; 