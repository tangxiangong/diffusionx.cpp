module;

#include <vector>
#include <stdexcept>
#include <string>

export module diffusionx.simulation.basic.moment;

import diffusionx.error;

using std::vector;

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
    int m_order; ///< The order of the moment
    T &process; ///< Reference to the process

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
    Moment(double duration, int order, T &process)
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
