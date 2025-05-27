module;

#include <memory>
#include <utility>
#include <vector>
#include <concepts>
#include <format>
#include <cmath>
#include <thread>
#include <stdexcept>
#include <string>
#include <optional>

export module diffusionx.simulation.basic;

import diffusionx.error;

using std::vector;
using std::format;

/**
 * @brief Type alias for a pair of vectors representing time and position data
 * 
 * This type is commonly used to represent simulation trajectories where the first
 * vector contains time points and the second vector contains corresponding positions.
 */
export using vec_pair = std::pair<vector<double>, vector<double>>;

/**
 * @brief Type alias for a pair of doubles representing a domain or interval
 * 
 * This type is used to represent intervals, domains, or ranges in simulations.
 */
export using double_pair = std::pair<double, double>;

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
    virtual Result<Option<double>> fpt(double_pair domain,
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
 * @brief Structure representing a continuous trajectory with a specific duration
 * @tparam T The type of the continuous process (must satisfy CP concept)
 * 
 * This structure encapsulates a continuous process with a fixed simulation duration.
 */
export template<CP T>
struct ContinuousTrajectory {
    double duration; ///< The simulation duration
    T& process;      ///< Reference to the continuous process

    /**
     * @brief Constructs a continuous trajectory with specified duration and process
     * @param duration The simulation duration
     * @param process Reference to the continuous process
     */
    ContinuousTrajectory(double duration, T& process)
        : duration(duration), process(process) {}
};

/**
 * @brief Template structure for computing moments of stochastic processes
 * @tparam T The type of the process
 * 
 * This structure provides methods to compute raw and central moments of
 * stochastic processes using Monte Carlo simulation.
 */
export template<typename T>
struct Moment {
    double m_duration; ///< The time at which to compute the moment
    int m_order;       ///< The order of the moment
    T& process;        ///< Reference to the process

    /**
     * @brief Default constructor
     */
    Moment() = default;
    
    /**
     * @brief Constructs a moment calculator with specified parameters
     * @param duration The time at which to compute the moment (must be positive)
     * @param order The order of the moment (must be non-negative)
     * @param process Reference to the process
     * @throws std::invalid_argument if duration <= 0 or order < 0
     */
    Moment(double duration, int order, T& process)
        : m_duration(duration), m_order(order), process(process) {
            if (duration <= 0) {
                throw std::invalid_argument("The duration must be greater than 0");
            }
            if (order < 0) {
                throw std::invalid_argument("The order must be greater than 0");
            }
        }

    /**
     * @brief Computes the raw moment using Monte Carlo simulation
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the raw moment value, or an Error
     */
    auto raw_moment(size_t particles = 10000, double time_step = 0.01) -> Result<double> {
        if (particles == 0) {
            return Err(Error::InvalidArgument("The number of particles must be greater than 0"));
        }
        if (time_step <= 0) {
            return Err(Error::InvalidArgument("The time step must be greater than 0"));
        }
        return Err(Error::NotImplemented("The method is not implemented"));
    }

    /**
     * @brief Computes the central moment using Monte Carlo simulation
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the central moment value, or an Error
     */
    auto central_moment(size_t particles = 10000, double time_step = 0.01) -> Result<double> {
        if (particles == 0) {
            return Err(Error::InvalidArgument("The number of particles must be greater than 0"));
        }
        if (time_step <= 0) {
            return Err(Error::InvalidArgument("The time step must be greater than 0"));
        }
        return Err(Error::NotImplemented("The method is not implemented"));
    }
};

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
    int m_order;       ///< The order of the moment
    T& process;        ///< Reference to the continuous process
    
    /**
     * @brief Computes the raw moment using parallel Monte Carlo simulation
     * @param particles The number of Monte Carlo samples (default: 10000)
     * @param time_step The time step for discretization (default: 0.01)
     * @return Result containing the raw moment value, or an Error
     */
    auto raw_moment(size_t particles = 10000, double time_step = 0.01) -> Result<double> {
        auto compute_func = [this, time_step](auto& proc) -> Result<double> {
            auto traj = proc.simulate(m_duration, time_step);
            if (!traj.has_value()) {
                return Err(traj.error());
            }
            auto [t, x] = traj.value();
            return std::pow(x[x.size() - 1], m_order);
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
        
        auto compute_func = [this, time_step, mean](auto& proc) -> Result<double> {
            auto traj = proc.simulate(m_duration, time_step);
            if (!traj.has_value()) {
                return Err(traj.error());
            }
            auto [t, x] = traj.value();
            return std::pow(x[x.size() - 1] - mean, m_order);
        };
        
        return parallel_monte_carlo(particles, process, compute_func);
    }
};

/**
 * @brief Performs parallel Monte Carlo simulation for statistical computations
 * @tparam ProcessType The type of the stochastic process
 * @tparam ComputeFunc The type of the computation function
 * @param particles The number of Monte Carlo samples
 * @param process Reference to the stochastic process
 * @param compute_func Function that computes a value from a single trajectory
 * @return Result containing the averaged result, or an Error
 * 
 * This function distributes Monte Carlo simulations across multiple threads
 * for improved performance. Each thread runs a subset of simulations and
 * the results are averaged.
 */
export template<typename ProcessType, typename ComputeFunc>
auto parallel_monte_carlo(size_t particles, ProcessType& process, ComputeFunc compute_func) -> Result<double> {
    if (particles == 0) {
        return Err(Error::InvalidArgument("The number of particles must be greater than 0"));
    }

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 1;
    }
    if (particles < num_threads) {
        num_threads = static_cast<unsigned int>(particles);
    }

    vector<std::thread> threads;
    threads.reserve(num_threads);
    
    vector<double> partial_results(num_threads, 0.0);
    vector<bool> success_flags(num_threads, true);
    vector<std::optional<Error>> errors(num_threads);

    size_t chunk_size = particles / num_threads;
    size_t remainder = particles % num_threads;

    for (unsigned int i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = start + chunk_size;
        if (i == num_threads - 1) {
            end += remainder;
        }

        threads.emplace_back([&, i, start, end]() {
            double local_sum = 0.0;
            for (size_t j = start; j < end; ++j) {
                auto result = compute_func(process);
                if (!result.has_value()) {
                    success_flags[i] = false;
                    errors[i] = result.error();
                    return;
                }
                local_sum += result.value();
            }
            partial_results[i] = local_sum;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Check for errors
    for (unsigned int i = 0; i < num_threads; ++i) {
        if (!success_flags[i]) {
            return Err(errors[i].value());
        }
    }

    // Sum up partial results
    double total_sum = 0.0;
    for (double partial : partial_results) {
        total_sum += partial;
    }

    return total_sum / particles;
}
