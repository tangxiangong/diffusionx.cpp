/**
 * @file continuous.cppm
 * @brief Continuous stochastic process simulation
 * 
 * This module provides the core infrastructure for simulating continuous stochastic processes.
 */

module;

#include <concepts>
#include <cmath>

export module diffusionx.simulation.basic.continuous;

import diffusionx.error;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.basic.moment;

/**
 * @brief Abstract base class for continuous stochastic processes
 * 
 * This class provides a common interface for all continuous stochastic processes.
 * It defines virtual methods for simulation, statistical analysis, and trajectory
 * properties that derived classes must implement.
 */
export struct ContinuousProcess {
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~ContinuousProcess() = default;

    /**
     * @brief Simulates a trajectory of the stochastic process
     * @param duration The total simulation time
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing a pair of time and position vectors, or an Error
     * 
     * This pure virtual method must be implemented by derived classes to generate
     * a single trajectory of the stochastic process.
     */
    virtual Result<vec_pair> simulate(double duration,
                                      double time_step = 0.01) = 0;

    /**
     * @brief Computes the mean of the process at a given time
     * @param duration The time at which to compute the mean
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the mean value, or an Error
     */
    virtual Result<double> mean(double duration, size_t particles = 10000,
                                double time_step = 0.01);

    /**
     * @brief Computes the mean squared displacement (MSD) at a given time
     * @param duration The time at which to compute the MSD
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the MSD value, or an Error
     */
    virtual Result<double> msd(double duration, size_t particles = 10000,
                               double time_step = 0.01);

    /**
     * @brief Computes the raw moment of the process at a given time
     * @param duration The time at which to compute the moment
     * @param order The order of the moment
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the raw moment value, or an Error
     */
    virtual Result<double> raw_moment(double duration, int order,
                                      size_t particles = 10000,
                                      double time_step = 0.01);

    /**
     * @brief Computes the central moment of the process at a given time
     * @param duration The time at which to compute the moment
     * @param order The order of the moment
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the central moment value, or an Error
     */
    virtual Result<double> central_moment(double duration, int order,
                                          size_t particles = 10000,
                                          double time_step = 0.01);

    /**
     * @brief Computes the first passage time (FPT) for a given domain
     * @param domain The domain boundaries as a pair (lower, upper)
     * @param max_duration Maximum simulation time (default: 1000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing an optional FPT (None if no passage occurs), or an Error
     */
    virtual Result<Option<double> > fpt(double_pair domain,
                                        double max_duration = 1000,
                                        double time_step = 0.01);

    /**
     * @brief Computes the occupation time within a given domain
     * @param domain The domain boundaries as a pair (lower, upper)
     * @param duration The total simulation time
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the occupation time, or an Error
     */
    virtual Result<double> occupation_time(double_pair domain, double duration,
                                           double time_step = 0.01);

    /**
     * @brief Computes the time-averaged mean squared displacement (TAMSD)
     * @param duration The total simulation time
     * @param delta The lag time for averaging
     * @param quad_order The order of quadrature for integration (default: 10)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the TAMSD value, or an Error
     */
    virtual Result<double> tamsd(double duration, double delta,
                                 size_t quad_order = 10,
                                 double time_step = 0.01);

    /**
     * @brief Computes the ensemble-averaged time-averaged MSD (EATAMSD)
     * @param duration The total simulation time
     * @param delta The lag time for averaging
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param quad_order The order of quadrature for integration (default: 10)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the EATAMSD value, or an Error
     */
    virtual Result<double> eatamsd(double duration, double delta,
                                   size_t particles = 10000,
                                   int quad_order = 10,
                                   double time_step = 0.01);
};

/**
 * @brief Concept defining types that derive from ContinuousProcess
 * @tparam T The type to check
 * 
 * This concept ensures that template parameters are derived from ContinuousProcess.
 */
export template<typename T>
concept CP = std::derived_from<T, ContinuousProcess>;

/**
 * @brief Specialized template for computing moments of continuous processes
 * @tparam T The type of the continuous process (must satisfy CP concept)
 * 
 * This specialization provides concrete implementations for computing moments
 * of continuous stochastic processes.
 */
template<CP T>
struct Moment<T> {
    double m_duration; ///< The time at which to compute the moment
    int m_order; ///< The order of the moment
    T &process; ///< Reference to the continuous process

    /**
     * @brief Computes the raw moment using parallel Monte Carlo simulation
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the raw moment value, or an Error
     */
    auto raw_moment(size_t particles = 10000, double time_step = 0.01) -> Result<double> {
        auto compute_func = [this, time_step](auto &proc) -> Result<double> {
            auto traj = proc.simulate(m_duration, time_step);
            if (!traj.has_value()) {
                return Err(traj.error());
            }
            auto [t, x] = traj.value();
            if (x.empty()) {
                return Err(Error::InvalidArgument("Empty trajectory"));
            }
            return std::pow(x.back(), m_order);
        };

        return parallel_monte_carlo(particles, process, compute_func);
    }

    /**
     * @brief Computes the central moment using parallel Monte Carlo simulation
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the central moment value, or an Error
     */
    auto central_moment(size_t particles = 10000, double time_step = 0.01) -> Result<double> {
        // First compute the mean using raw moment of order 1
        Moment<T> mean_moment(m_duration, 1, process);
        auto mean_result = mean_moment.raw_moment(particles, time_step);
        if (!mean_result.has_value()) {
            return Err(mean_result.error());
        }
        double mean = mean_result.value();

        auto compute_func = [this, time_step, mean](auto &proc) -> Result<double> {
            auto traj = proc.simulate(m_duration, time_step);
            if (!traj.has_value()) {
                return Err(traj.error());
            }
            auto [t, x] = traj.value();
            if (x.empty()) {
                return Err(Error::InvalidArgument("Empty trajectory"));
            }

            return std::pow(x.back() - mean, m_order);
        };

        return parallel_monte_carlo(particles, process, compute_func);
    }
};

Result<double> ContinuousProcess::mean(double duration, size_t particles, double time_step) {
    auto moment = Moment<ContinuousProcess>(duration, 1, *this);
    return moment.raw_moment(particles, time_step);
}

Result<double> ContinuousProcess::msd(double duration, size_t particles, double time_step) {
    auto moment = Moment<ContinuousProcess>(duration, 2, *this);
    return moment.central_moment(particles, time_step);
}

auto ContinuousProcess::raw_moment(double duration, int order, size_t particles, double time_step) -> Result<double> {
    auto moment = Moment<ContinuousProcess>(duration, order, *this);
    return moment.raw_moment(particles, time_step);
}

auto ContinuousProcess::central_moment(double duration, int order, size_t particles,
                                       double time_step) -> Result<double> {
    auto moment = Moment<ContinuousProcess>(duration, order, *this);
    return moment.central_moment(particles, time_step);
}
