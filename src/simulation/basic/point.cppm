module;

#include <cmath>
#include <vector>
#include <random>
#include <algorithm>

export module diffusionx.simulation.point_process;

import diffusionx.error;
import diffusionx.random.exponential;
import diffusionx.random.poisson;
import diffusionx.simulation.basic.utils;

using std::vector;

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
    virtual Result<vector<double>> simulate(double duration) = 0;

    /**
     * @brief Simulates the point process and returns trajectory
     * @param duration The total simulation time
     * @return Result containing time and position vectors, or an Error
     */
    virtual Result<vec_pair> simulate_trajectory(double duration) = 0;
};

/**
 * @brief Poisson process implementation
 * 
 * A Poisson process is a point process where events occur independently
 * at a constant average rate λ. The inter-arrival times follow an
 * exponential distribution with parameter λ.
 */
export class PoissonProcess : public PointProcess {
    double m_rate = 1.0;  ///< Event rate λ

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
    [[nodiscard]] auto get_rate() const -> double {
        return m_rate;
    }

    /**
     * @brief Simulates the Poisson process
     * @param duration The total simulation time
     * @return Result containing event times, or an Error
     */
    Result<vector<double>> simulate(double duration) override {
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
     * @brief Simulates the Poisson process and returns counting process trajectory
     * @param duration The total simulation time
     * @return Result containing time and count vectors, or an Error
     */
    Result<vec_pair> simulate_trajectory(double duration) override {
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

/**
 * @brief Birth-death process implementation
 * 
 * A birth-death process is a continuous-time Markov chain where
 * the state can only increase by 1 (birth) or decrease by 1 (death).
 * The birth rate is λ and the death rate is μ.
 */
export class BirthDeathProcess : public PointProcess {
    double m_birth_rate = 1.0;   ///< Birth rate λ
    double m_death_rate = 0.5;   ///< Death rate μ
    int m_initial_state = 0;     ///< Initial population

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
    BirthDeathProcess(double birth_rate, double death_rate, int initial_state = 0)
        : m_birth_rate(birth_rate), m_death_rate(death_rate), m_initial_state(initial_state) {
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
    [[nodiscard]] auto get_birth_rate() const -> double {
        return m_birth_rate;
    }

    /**
     * @brief Gets the death rate
     * @return The death rate μ
     */
    [[nodiscard]] auto get_death_rate() const -> double {
        return m_death_rate;
    }

    /**
     * @brief Gets the initial state
     * @return The initial population
     */
    [[nodiscard]] auto get_initial_state() const -> int {
        return m_initial_state;
    }

    /**
     * @brief Simulates event times (not directly applicable for birth-death)
     * @param duration The total simulation time
     * @return Result containing transition times, or an Error
     */
    Result<vector<double>> simulate(double duration) override {
        auto trajectory_result = simulate_trajectory(duration);
        if (!trajectory_result.has_value()) {
            return Err(trajectory_result.error());
        }
        auto [times, states] = trajectory_result.value();
        
        // Return transition times (excluding initial time)
        vector<double> transition_times(times.begin() + 1, times.end());
        return Ok(std::move(transition_times));
    }

    /**
     * @brief Simulates the birth-death process trajectory
     * @param duration The total simulation time
     * @return Result containing time and state vectors, or an Error
     */
    Result<vec_pair> simulate_trajectory(double duration) override {
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
            // Calculate total rate
            double total_rate = m_birth_rate * current_state + m_death_rate * current_state;
            
            if (total_rate <= 0.0) {
                // No transitions possible (extinct population)
                break;
            }

            // Generate time to next event
            std::exponential_distribution<double> exp_dist(total_rate);
            double time_to_next = exp_dist(gen);
            current_time += time_to_next;

            if (current_time >= duration) {
                break;
            }

            // Determine type of event
            double u = uniform_dist(gen);
            double birth_prob = (m_birth_rate * current_state) / total_rate;

            if (u < birth_prob) {
                // Birth event
                current_state++;
            } else {
                // Death event
                current_state--;
                if (current_state < 0) {
                    current_state = 0;  // Prevent negative population
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
};

/**
 * @brief Compound Poisson process implementation
 * 
 * A compound Poisson process is a sum of independent and identically
 * distributed random variables, where the number of terms follows
 * a Poisson distribution.
 */
export class CompoundPoissonProcess : public PointProcess {
    double m_rate = 1.0;           ///< Event rate λ
    double m_jump_mean = 1.0;      ///< Mean jump size
    double m_jump_std = 1.0;       ///< Jump size standard deviation

public:
    /**
     * @brief Default constructor
     */
    CompoundPoissonProcess() = default;

    /**
     * @brief Constructs a compound Poisson process
     * @param rate Event rate λ (must be positive)
     * @param jump_mean Mean jump size
     * @param jump_std Jump size standard deviation (must be positive)
     * @throws std::invalid_argument if parameters are invalid
     */
    CompoundPoissonProcess(double rate, double jump_mean, double jump_std)
        : m_rate(rate), m_jump_mean(jump_mean), m_jump_std(jump_std) {
        if (m_rate <= 0.0) {
            throw std::invalid_argument("Rate must be positive");
        }
        if (m_jump_std <= 0.0) {
            throw std::invalid_argument("Jump standard deviation must be positive");
        }
    }

    /**
     * @brief Gets the event rate
     * @return The event rate λ
     */
    [[nodiscard]] auto get_rate() const -> double {
        return m_rate;
    }

    /**
     * @brief Gets the jump mean
     * @return The mean jump size
     */
    [[nodiscard]] auto get_jump_mean() const -> double {
        return m_jump_mean;
    }

    /**
     * @brief Gets the jump standard deviation
     * @return The jump size standard deviation
     */
    [[nodiscard]] auto get_jump_std() const -> double {
        return m_jump_std;
    }

    /**
     * @brief Simulates event times
     * @param duration The total simulation time
     * @return Result containing event times, or an Error
     */
    Result<vector<double>> simulate(double duration) override {
        PoissonProcess poisson(m_rate);
        return poisson.simulate(duration);
    }

    /**
     * @brief Simulates the compound Poisson process trajectory
     * @param duration The total simulation time
     * @return Result containing time and cumulative sum vectors, or an Error
     */
    Result<vec_pair> simulate_trajectory(double duration) override {
        if (duration <= 0) {
            return Err(Error::InvalidArgument("Duration must be positive"));
        }

        auto events_result = simulate(duration);
        if (!events_result.has_value()) {
            return Err(events_result.error());
        }
        auto event_times = events_result.value();

        vector<double> times;
        vector<double> values;

        times.push_back(0.0);
        values.push_back(0.0);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> normal_dist(m_jump_mean, m_jump_std);

        double cumulative_sum = 0.0;
        for (double event_time : event_times) {
            double jump_size = normal_dist(gen);
            cumulative_sum += jump_size;
            
            times.push_back(event_time);
            values.push_back(cumulative_sum);
        }

        times.push_back(duration);
        values.push_back(cumulative_sum);

        return Ok(std::make_pair(std::move(times), std::move(values)));
    }
}; 