module;

#include <algorithm>
#include <vector>

export module diffusionx.simulation.continuous.brownian_bridge;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.continuous.bm;

using std::vector;

export Result<vec_pair> simulate_brownian_bridge(double duration,
                                                 double time_step) {
    if (auto result = check_duration_time_step(duration, time_step); !result) {
        return Err(result.error());
    }
    Bm bm{};
    auto bm_result = bm.simulate(duration, time_step);
    if (!bm_result) {
        return Err(bm_result.error());
    }
    auto [t, traj] = bm_result.value();
    double end_position = traj.back();

    vector<double> x(t.size());
    for (size_t i = 0; i < t.size(); ++i) {
        x[i] = traj[i] - end_position * t[i] / duration;
    }
    return Ok(std::make_pair(std::move(t), std::move(x)));
}

using std::vector;

/**
 * @brief Brownian bridge
 */
export class BrownianBridge final : public ContinuousProcess {
  public:
    BrownianBridge() = default;

    double start() override { return 0; }

    /**
     * @brief Simulates a trajectory of the Brownian bridge
     * @param duration The total simulation time (should equal total_time)
     * @param time_step The time step for discretization
     * @return Result containing time and position vectors, or an Error
     *
     * Uses the sequential conditioning method for exact simulation.
     */
    Result<vec_pair> simulate(double duration, double time_step) override {
        return simulate_brownian_bridge(duration, time_step);
    }

    Result<double> displacement(double duration, double time_step) override {
        return Ok(0.0);
    }
};
