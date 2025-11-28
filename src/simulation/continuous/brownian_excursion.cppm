module;

#include <algorithm>
#include <cmath>
#include <ranges>
#include <vector>

export module diffusionx.simulation.continuous.brownian_excursion;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.continuous.brownian_bridge;

using std::vector;

export Result<vec_pair> simulate_brownian_excursion(double duration,
                                                    double time_step) {
    if (auto result = check_duration_time_step(duration, time_step); !result) {
        return Err(result.error());
    }
    if (duration > 1.0) {
        return Err(Error::InvalidArgument(
            "Duration must be less than or equal to 1 for Brownian excursion"));
    }

    BrownianBridge bridge{};
    auto bridge_result = bridge.simulate(duration, time_step);
    if (!bridge_result) {
        return Err(bridge_result.error());
    }
    auto [t, bridge_traj] = bridge_result.value();

    auto min_iter = std::ranges::min_element(bridge_traj);
    auto min_iter_idx = std::distance(bridge_traj.begin(), min_iter);
    double min_traj = *min_iter;
    double tau_m = t[min_iter_idx];

    double tt;
    std::ranges::borrowed_iterator_t<vector<double> &> it;
    size_t index;
    vector<double> x(t.size());
    for (size_t i = 0; i < t.size(); ++i) {
        tt = (t[i] + tau_m) - std::floor((t[i] + tau_m));
        it = std::ranges::find_if(t, [tt](int v) { return v >= tt; });
        index = std::ranges::distance(t.begin(), it);
        x[i] = bridge_traj[index] - min_traj;
    }

    return Ok(std::make_pair(std::move(t), std::move(x)));
}

/**
 * @brief Brownian excursion
 *
 */
export class BrownianExcursion final : public ContinuousProcess {
  public:
    BrownianExcursion() = default;

    /**
     * @brief Simulates a trajectory of the Brownian excursion
     * @param duration The total simulation time (should equal total_time)
     * @param time_step The time step for discretization
     * @return Result containing time and position vectors, or an Error
     *
     * Uses rejection sampling on Brownian bridges.
     */
    Result<vec_pair> simulate(double duration, double time_step) override {
        return simulate_brownian_excursion(duration, time_step);
    }

    double start() override { return 0.0; }
};
