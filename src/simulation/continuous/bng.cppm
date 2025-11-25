module;

#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.bng;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::vector;

export Result<vec_pair> simulate_bng(double start_position,
                                     double ou_start_position, double duration,
                                     double time_step) {
    if (auto result = check_duration_time_step(duration, time_step); !result) {
        return Err(result.error());
    }

    auto num_steps = static_cast<size_t>(std::ceil(duration / time_step));
    vector<double> t(num_steps + 1);
    vector<double> x(num_steps + 1);

    t[0] = 0.0;
    x[0] = start_position;

    double current_t = 0.0;
    double current_x = start_position;
    double current_y = ou_start_position;

    double scale_ou = std::sqrt(time_step);
    double scale_bng = std::sqrt(2.0 * time_step);

    auto noises_ou = randn(num_steps - 1).value();
    auto noises_bgn = randn(num_steps - 1).value();

    for (size_t i = 0; i < num_steps - 1; ++i) {
        current_t += time_step;
        current_y += -current_y * time_step + noises_ou[i] * scale_ou;
        current_x += std::abs(current_y) * noises_bgn[i] * scale_bng;
        t[i + 1] = current_t;
        x[i + 1] = current_x;
    }

    double last_step = duration - current_t;
    scale_ou = std::sqrt(last_step);
    scale_bng = std::sqrt(2.0 * last_step);

    current_y += -current_y * last_step + randn() * scale_ou;
    current_x += std::abs(current_y) * randn() * scale_bng;

    t[num_steps] = duration;
    x[num_steps] = current_x;

    return Ok(std::make_pair(std::move(t), std::move(x)));
}

/**
 * @brief Brownian yet non-Gaussian (BnG) process implementation
 *
 * The BnG process is a stochastic process that exhibits Brownian scaling
 * (mean square displacement ~ t) but has non-Gaussian displacement
 * distributions. This is achieved by coupling a diffusion process with a
 * time-varying diffusion coefficient driven by an Ornstein-Uhlenbeck process.
 *
 * Mathematical definition:
 * dr(t) = √(2D(t)) dW₁(t), r(0) = r₀
 * D(t) = Y(t)²
 * dY(t) = -Y(t) dt + dW₂(t), Y(0) = Y₀
 *
 * where W₁(t) and W₂(t) are two independent Wiener processes.
 *
 * Properties:
 * - Linear mean square displacement: ⟨r²(t)⟩ ~ t
 * - Non-Gaussian displacement distributions
 * - Exponential tails in displacement probability density
 */
export class BnG final : public ContinuousProcess {
    double m_start_position = 0.0;    ///< Initial position r₀
    double m_ou_start_position = 0.0; ///< Initial OU process value Y₀

  public:
    /**
     * @brief Default constructor creating standard BnG process
     */
    BnG() = default;

    /**
     * @brief Constructs BnG process with specified parameters
     * @param start_position Initial position r₀
     * @param ou_start_position Initial OU process value Y₀
     */
    BnG(double start_position, double ou_start_position)
        : m_start_position(start_position),
          m_ou_start_position(ou_start_position) {}

    /**
     * @brief Gets the initial position
     * @return The initial position r₀
     */
    [[nodiscard]] auto get_start_position() const -> double {
        return m_start_position;
    }

    /**
     * @brief Gets the initial OU process value
     * @return The initial OU process value Y₀
     */
    [[nodiscard]] auto get_ou_start_position() const -> double {
        return m_ou_start_position;
    }

    /**
     * @brief Simulates a trajectory of the BnG process
     * @param duration The total simulation time
     * @param time_step The time step for discretization
     * @return Result containing time and position vectors, or an Error
     *
     * Algorithm:
     * 1. Simulate OU process Y(t) with θ = 1, σ = 1
     * 2. Use |Y(t)| as time-varying diffusion coefficient
     * 3. Generate position increments with √(2|Y(t)|dt) * Z
     */
    Result<vec_pair> simulate(double duration, double time_step) override {
        return simulate_bng(m_start_position, m_ou_start_position, duration,
                            time_step);
    }

    double start() override { return m_start_position; }

    Result<double> displacement(double duration, double time_step) override {
        if (auto result = check_duration_time_step(duration, time_step); !result) {
            return Err(result.error());
        }

        auto num_steps = static_cast<size_t>(std::ceil(duration / time_step));

        double current_x = m_start_position;
        double current_y = m_ou_start_position;

        double scale_ou = std::sqrt(time_step);
        double scale_bng = std::sqrt(2.0 * time_step);

        auto noises_ou = randn(num_steps - 1).value();
        auto noises_bgn = randn(num_steps - 1).value();

        for (size_t i = 0; i < num_steps - 1; ++i) {
            current_y += -current_y * time_step + noises_ou[i] * scale_ou;
            current_x += std::abs(current_y) * noises_bgn[i] * scale_bng;
        }

        double last_step = duration - static_cast<double>(num_steps - 1) * time_step;
        scale_ou = std::sqrt(last_step);
        scale_bng = std::sqrt(2.0 * last_step);

        current_y += -current_y * last_step + randn() * scale_ou;
        current_x += std::abs(current_y) * randn() * scale_bng;

        return Ok(current_x - m_start_position);
    }
};
