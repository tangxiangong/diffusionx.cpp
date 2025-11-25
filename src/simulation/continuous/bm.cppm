module;

#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.bm;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

export Result<vec_pair> simulate_bm(double start_position, double diffusion_coefficient, double duration, double time_step) {
    if (auto result = check_duration_time_step(duration, time_step); !result) {
        return Err(result.error());
    }

    auto num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> times(num_steps + 1);
    vector<double> positions(num_steps + 1);

    double current_t = 0.0;
    double current_x = start_position;

    // Initialize
    times[0] = 0.0;
    positions[0] = start_position;

    // Generate increments
    auto increments =
        randn(num_steps - 1, 0.0,
              std::sqrt(2.0 * diffusion_coefficient * time_step)).value();


    for (size_t i = 1; i < num_steps; ++i) {
        current_t += time_step;
        current_x += increments[i - 1];
        times[i] = current_t;
        positions[i] = current_x;
    }

    double last_step = duration - current_t;
    double increment =
        randn(0.0, std::sqrt(2.0 * diffusion_coefficient * last_step))
            .value();
    current_x += increment;
    times.back() = duration;
    positions.back() = current_x;

    return Ok(std::make_pair(std::move(times), std::move(positions)));
}

/**
 * @brief Brownian motion (Wiener process) implementation
 *
 * This class implements the standard Brownian motion, a continuous-time
 * stochastic process with independent, normally distributed increments. The
 * process satisfies:
 * - B(0) = start_position
 * - B(t) - B(s) ~ N(0, diffusion_coefficient * |t - s|) for t > s
 *
 * Mathematical definition:
 * dX(t) = √(2D) dW(t)
 *
 * where D is the diffusion coefficient and W(t) is a standard Wiener process.
 */
export class Bm final : public ContinuousProcess {
    double m_start_position = 0.0;        ///< Initial position of the process
    double m_diffusion_coefficient = 0.5; ///< Diffusion coefficient (D)

  public:
    /**
     * @brief Default constructor creating standard Brownian motion
     *
     * Creates a Brownian motion starting at position 0 with unit diffusion
     * coefficient.
     */
    Bm() = default;

    /**
     * @brief Constructs Brownian motion with specified parameters
     * @param start_position Initial position of the process
     * @param diffusion_coefficient Diffusion coefficient (must be positive)
     * @throws std::invalid_argument if diffusion_coefficient is not positive
     */
    Bm(double start_position, double diffusion_coefficient)
        : m_start_position(start_position),
          m_diffusion_coefficient(diffusion_coefficient) {
        if (m_diffusion_coefficient <= 0) {
            throw std::invalid_argument(
                "Diffusion coefficient must be positive");
        }
    }

    /**
     * @brief Gets the initial position
     * @return The initial position of the process
     */
    [[nodiscard]] auto get_start_position() const -> double {
        return m_start_position;
    }

    /**
     * @brief Gets the diffusion coefficient
     * @return The diffusion coefficient
     */
    [[nodiscard]] auto get_diffusion_coefficient() const -> double {
        return m_diffusion_coefficient;
    }

    double start() override { return m_start_position; }

    /**
     * @brief Simulates a trajectory of the Brownian motion
     * @param duration The total simulation time
     * @param time_step The time step for discretization
     * @return Result containing time and position vectors, or an Error
     *
     * Uses the Euler-Maruyama scheme for simulation:
     * X(t + dt) = X(t) + √(2D * dt) * Z
     * where Z ~ N(0, 1)
     */
    Result<vec_pair> simulate(double duration, double time_step) override {
        return simulate_bm(m_start_position, m_diffusion_coefficient, duration, time_step);
    }

    Result<double> displacement(double duration, double time_step) override {
        if (auto result = check_duration_time_step(duration, time_step); !result) {
            return Err(result.error());
        }

        auto num_steps = static_cast<size_t>(std::ceil(duration / time_step));
        double current_x = m_start_position;

        // Generate increments
        auto increments =
            randn(num_steps - 1, 0.0,
                  std::sqrt(2.0 * m_diffusion_coefficient * time_step)).value();
        // Simulate trajectory
        for (const auto increment : increments) {
            current_x += increment;
        }

        double last_step =
            duration - static_cast<double>(num_steps - 1) * time_step;
        double increment =
            randn(0.0, std::sqrt(2.0 * m_diffusion_coefficient * last_step))
                .value();
        current_x += increment;

        return Ok(current_x - m_start_position);
    }
};
