module;

#include <algorithm>
#include <cmath>
#include <concepts>
#include <format>
#include <string>
#include <vector>

using std::vector;

export module diffusionx.simulation.basic.abstract;

import diffusionx.error;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.basic.moment;
import diffusionx.simulation.basic.functional;

/**
 * @brief Abstract base class for continuous stochastic processes
 *
 * This class provides a common interface for all continuous stochastic
 * processes. It defines virtual methods for simulation, statistical analysis,
 * and trajectory properties that derived classes must implement.
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
     * @return Result containing an optional FPT (None if no passage occurs), or
     * an Error
     */
    virtual Result<Option<double> >
    fpt(double_pair domain, double max_duration = 1000, double time_step = 0.01);

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
                                 size_t quad_order = 10, double time_step = 0.01);

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
                                   size_t particles = 10000, int quad_order = 10,
                                   double time_step = 0.01);
};

/**
 * @brief Concept defining types that derive from ContinuousProcess
 * @tparam T The type to check
 *
 * This concept ensures that template parameters are derived from
 * ContinuousProcess.
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
    auto raw_moment(size_t particles = 10000, double time_step = 0.01)
        -> Result<double> {
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
    auto central_moment(size_t particles = 10000, double time_step = 0.01)
        -> Result<double> {
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

Result<double> ContinuousProcess::mean(double duration, size_t particles,
                                       double time_step) {
    auto moment = Moment<ContinuousProcess>(duration, 1, *this);
    return moment.raw_moment(particles, time_step);
}

Result<double> ContinuousProcess::msd(double duration, size_t particles,
                                      double time_step) {
    auto moment = Moment<ContinuousProcess>(duration, 2, *this);
    return moment.central_moment(particles, time_step);
}

auto ContinuousProcess::raw_moment(double duration, int order, size_t particles,
                                   double time_step) -> Result<double> {
    auto moment = Moment<ContinuousProcess>(duration, order, *this);
    return moment.raw_moment(particles, time_step);
}

auto ContinuousProcess::central_moment(double duration, int order,
                                       size_t particles, double time_step)
    -> Result<double> {
    auto moment = Moment<ContinuousProcess>(duration, order, *this);
    return moment.central_moment(particles, time_step);
}

Result<Option<double>> ContinuousProcess::fpt(double_pair domain,
                                               double max_duration,
                                               double time_step) {
    auto fpt_obj = FirstPassageTime(*this, domain);
    return fpt_obj.simulate(max_duration, time_step);
}

Result<double> ContinuousProcess::occupation_time(double_pair domain,
                                                   double duration,
                                                   double time_step) {
    return Err(Error::NotImplemented(
        "occupation_time is not implemented for this process"));
}

Result<double> ContinuousProcess::tamsd(double duration, double delta,
                                        size_t quad_order, double time_step) {
    return Err(Error::NotImplemented(
        "tamsd is not implemented for this process"));
}

Result<double> ContinuousProcess::eatamsd(double duration, double delta,
                                          size_t particles, int quad_order,
                                          double time_step) {
    return Err(Error::NotImplemented(
        "eatamsd is not implemented for this process"));
}

/**
 * @brief Base class for point processes
 *
 * A point process is a random collection of points in time or space.
 * This base class provides the interface for simulating point processes.
 */
export class PointProcess {
public:
    virtual ~PointProcess() = default;

    /**
     * @brief Simulates the point process over a given duration
     * @param duration The total simulation time
     * @return Result containing event times, or an Error
     */
    virtual Result<vec_pair> simulate(double duration);

    /**
     * @brief Simulates the point process and returns trajectory
     * @param num_steps The number of steps.
     * @return Result containing time and position vectors, or an Error
     */
    virtual Result<vec_pair> simulate_with_step(size_t num_steps) = 0;

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
     * @return Result containing an optional FPT (None if no passage occurs), or
     * an Error
     */
    virtual Result<Option<double> >
    fpt(double_pair domain, double max_duration = 1000, double time_step = 0.01);

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
                                 size_t quad_order = 10, double time_step = 0.01);

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
                                   size_t particles = 10000, int quad_order = 10,
                                   double time_step = 0.01);
};

/**
 * @brief Concept defining types that derive from `PointProcess`
 * @tparam T The type to check
 *
 * This concept ensures that template parameters are derived from
 * `PointProcess`.
 */
export template<typename T>
concept Point = std::derived_from<T, PointProcess>;

/**
 * @brief Specialized template for computing moments of point processes
 * @tparam T The type of the continuous process (must satisfy `Point` concept)
 *
 * This specialization provides concrete implementations for computing moments
 * of point stochastic processes.
 */
template<Point T>
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
    auto raw_moment(size_t particles = 10000, double time_step = 0.01)
        -> Result<double> {
        auto compute_func = [this](auto &proc) -> Result<double> {
            auto traj = proc.simulate(m_duration);
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
    auto central_moment(size_t particles = 10000, double time_step = 0.01)
        -> Result<double> {
        // First compute the mean using raw moment of order 1
        Moment<T> mean_moment(m_duration, 1, process);
        auto mean_result = mean_moment.raw_moment(particles, time_step);
        if (!mean_result.has_value()) {
            return Err(mean_result.error());
        }
        double mean = mean_result.value();

        auto compute_func = [this, mean](auto &proc) -> Result<double> {
            auto traj = proc.simulate(m_duration);
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

Result<double> PointProcess::mean(double duration, size_t particles,
                                  double time_step) {
    auto moment = Moment<PointProcess>(duration, 1, *this);
    return moment.raw_moment(particles, time_step);
}

Result<double> PointProcess::msd(double duration, size_t particles,
                                 double time_step) {
    auto moment = Moment<PointProcess>(duration, 2, *this);
    return moment.central_moment(particles, time_step);
}

auto PointProcess::raw_moment(double duration, int order, size_t particles,
                              double time_step) -> Result<double> {
    auto moment = Moment<PointProcess>(duration, order, *this);
    return moment.raw_moment(particles, time_step);
}

auto PointProcess::central_moment(double duration, int order, size_t particles,
                                  double time_step) -> Result<double> {
    auto moment = Moment<PointProcess>(duration, order, *this);
    return moment.central_moment(particles, time_step);
}

Result<Option<double>> PointProcess::fpt(double_pair domain,
                                          double max_duration,
                                          double time_step) {
    auto fpt_obj = FirstPassageTime(*this, domain);
    return fpt_obj.simulate(max_duration, time_step);
}

Result<double> PointProcess::occupation_time(double_pair domain,
                                              double duration,
                                              double time_step) {
    return Err(Error::NotImplemented(
        "occupation_time is not implemented for this process"));
}

Result<double> PointProcess::tamsd(double duration, double delta,
                                   size_t quad_order, double time_step) {
    return Err(Error::NotImplemented(
        "tamsd is not implemented for this process"));
}

Result<double> PointProcess::eatamsd(double duration, double delta,
                                     size_t particles, int quad_order,
                                     double time_step) {
    return Err(Error::NotImplemented(
        "eatamsd is not implemented for this process"));
}

auto PointProcess::simulate(double duration) -> Result<vec_pair> {
    auto num_step = static_cast<size_t>(std::ceil(duration));
    auto tx = std::make_pair(vector<double>(), vector<double>());
    while (true) {
        auto result = this->simulate_with_step(num_step);
        if (!result.has_value()) {
            return Err(result.error());
        }
        tx.swap(result.value());
        auto [t, x] = tx;
        if (double end_time = t.back(); end_time >= duration) {
            break;
        }
        num_step *= 2;
    }
    auto [t, x] = tx;
    auto it = std::ranges::find_if(
        t, [duration](double time) { return time >= duration; });
    size_t index = std::distance(t.begin(), it);
    vector<double> t_{};
    vector<double> x_{};
    t_.resize(index + 1);
    x_.resize(index + 1);
    std::copy_n(t.begin(), index, t_.begin());
    std::copy_n(x.begin(), index, x_.begin());
    if (t[index] > duration) {
        t_[index] = duration;
        x_[index] = x_[index - 1];
    } else {
        t_[index] = t[index];
        x_[index] = x[index];
    }

    return Ok(std::make_pair(t_, x_));
}

/**
 * @brief Type alias for discrete trajectory data (step numbers, positions)
 */
export using discrete_pair = std::pair<vector<size_t>, vector<double> >;

/**
 * @brief Abstract base class for discrete stochastic processes
 *
 * This class provides a common interface for all discrete stochastic processes.
 * It defines virtual methods for simulation, statistical analysis, and
 * trajectory properties that derived classes must implement.
 */
export struct DiscreteProcess {
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~DiscreteProcess() = default;

    /**
     * @brief Simulates a trajectory of the discrete stochastic process
     * @param num_steps The number of steps to simulate
     * @return Result containing a pair of step numbers and position vectors, or
     * an Error
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
    virtual Result<double> raw_moment(size_t num_steps, int order,
                                      size_t particles = 10000);

    /**
     * @brief Computes the central moment of the process at a given step
     * @param num_steps The number of steps
     * @param order The order of the moment
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @return Result containing the central moment value, or an Error
     */
    virtual Result<double> central_moment(size_t num_steps, int order,
                                          size_t particles = 10000);
};

/**
 * @brief Concept defining types that derive from DiscreteProcess
 * @tparam T The type to check
 *
 * This concept ensures that template parameters are derived from
 * DiscreteProcess.
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

auto DiscreteProcess::raw_moment(size_t num_steps, int order, size_t particles)
    -> Result<double> {
    auto moment = Moment<DiscreteProcess>(num_steps, order, *this);
    return moment.raw_moment(particles);
}

auto DiscreteProcess::central_moment(size_t num_steps, int order,
                                     size_t particles) -> Result<double> {
    auto moment = Moment<DiscreteProcess>(num_steps, order, *this);
    return moment.central_moment(particles);
}

template<CP T>
class FirstPassageTime<T> {
private:
    T &m_process; ///< Reference to the stochastic process
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

    Result<Option<double> > simulate(double max_duration,
                                     double time_step = 0.01) {
        if (max_duration <= 0) {
            return Err(Error::InvalidArgument("max_duration must be positive"));
        }
        if (time_step <= 0) {
            return Err(Error::InvalidArgument("time_step must be positive"));
        }
        if (max_duration < time_step) {
            return Err(Error::InvalidArgument(
                "max_duration must be greater than time_step"));
        }
        auto [a, b] = m_domain;
        auto find = [a, b](const vector<double> &v) {
            auto it = std::ranges::find_if(v, [a, b](double position) {
                return position <= a || position >= b;
            });
            if (it != v.end()) {
                size_t idx = std::distance(v.begin(), it);
                return Some(idx);
            }
            return std::nullopt;
        };
        double duration = std::max(max_duration / 10, 10.0);
        while (true) {
            auto traj_result = m_process.simulate(duration, time_step);
            if (!traj_result.has_value()) {
                return Err(traj_result.error());
            }
            auto [t, x] = traj_result.value();
            if (auto find_result = find(&x); find_result.has_value()) {
                return Ok(find_result);
            }
            duration *= 2;
            if (duration > max_duration) {
                duration = max_duration;
                traj_result = m_process.simulate(duration, time_step);
                if (!traj_result.has_value()) {
                    return Err(traj_result.error());
                }
                auto [t, x] = traj_result.value();
                if (auto find_result = find(&x); find_result.has_value()) {
                    return Ok(find_result);
                }
                return Ok(std::nullopt);
            }
        }
    }
};
