module;

#include <algorithm>
#include <cmath>
#include <vector>

export module diffusionx.simulation.continuous.brownian_meander;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;
import diffusionx.simulation.continuous.bm;

using std::vector;

export Result<vec_pair> simulate_brownian_meander(double duration,
                                                  double time_step) {
    if (auto result = check_duration_time_step(duration, time_step); !result) {
        return Err(result.error());
    }

    if (duration > 1.0) {
        return Err(Error::InvalidArgument(
            "Duration must be less than or equal to 1 for Brownian meander"));
    }

    Bm bm{};
    auto bm_result = bm.simulate(duration, time_step);
    if (!bm_result) {
        return Err(bm_result.error());
    }
    auto [bm_t, bm_traj] = bm_result.value();

    vector<size_t> hint_indexes;
    for (size_t i = 0; i < bm_traj.size(); ++i) {
        if (std::abs(bm_traj[i]) < 1e-10) {
            hint_indexes.push_back(i);
        }
    }
    size_t last_hint_index = hint_indexes.empty() ? 0 : hint_indexes.back();
    double tau = (last_hint_index == bm_traj.size() - 1)
                     ? 1.0 - time_step
                     : bm_t[last_hint_index];
    double coe = 1.0 / std::sqrt(1.0 - tau);

    vector<double> x(bm_t.size());
    double time;
    std::ranges::borrowed_iterator_t<vector<double> &> it;
    size_t right_index;
    size_t left_index;
    double left_time;
    double right_time;
    double left_traj;
    double right_traj;
    double k;
    double value;
    for (size_t i = 0; i < bm_t.size(); ++i) {
        time = bm_t[i] * (1.0 - tau) + tau;
        it = std::ranges::find_if(bm_t, [time](double v) { return v > time; });
        right_index = it == bm_t.end()
                          ? bm_t.size() - 1
                          : std::ranges::distance(bm_t.begin(), it);
        left_index = right_index - 1;
        left_time = bm_t[left_index];
        right_time = bm_t[right_index];
        left_traj = bm_traj[left_index];
        right_traj = bm_traj[right_index];
        k = (left_traj - right_traj) / (left_time - right_time);
        value = k * (time - left_time) + left_traj;
        x[i] = std::abs(value) * coe;
    }

    return Ok(std::make_pair(std::move(bm_t), std::move(x)));
}

/**
 * @brief Brownian meander
 */
export class BrownianMeander final : public ContinuousProcess {
  public:
    /**
     * @brief Default constructor creating a standard Brownian meander
     *
     * Creates a Brownian meander over time interval [0, 1].
     */
    BrownianMeander() = default;

    /**
     * @brief Simulates a trajectory of the Brownian meander
     * @param duration The total simulation time (should equal total_time)
     * @param time_step The time step for discretization
     * @return Result containing time and position vectors, or an Error
     *
     * Uses the reflection principle and rejection sampling.
     */
    Result<vec_pair> simulate(double duration, double time_step) override {
        return simulate_brownian_meander(duration, time_step);
    }

    double start() override { return 0.0; }

    Result<double> displacement(double duration, double time_step) override {
        if (auto result = check_duration_time_step(duration, time_step);
            !result) {
            return Err(result.error());
        }

        if (duration > 1.0) {
            return Err(
                Error::InvalidArgument("Duration must be less than or equal to "
                                       "1 for Brownian meander"));
        }

        Bm bm{};
        auto bm_result = bm.simulate(duration, time_step);
        if (!bm_result) {
            return Err(bm_result.error());
        }
        auto [bm_t, bm_traj] = bm_result.value();

        vector<size_t> hint_indexes;
        for (size_t i = 0; i < bm_traj.size(); ++i) {
            if (std::abs(bm_traj[i]) < 1e-10) {
                hint_indexes.push_back(i);
            }
        }
        size_t last_hint_index = hint_indexes.empty() ? 0 : hint_indexes.back();
        double tau = (last_hint_index == bm_traj.size() - 1)
                         ? 1.0 - time_step
                         : bm_t[last_hint_index];
        double coe = 1.0 / std::sqrt(1.0 - tau);

        double time;
        std::ranges::borrowed_iterator_t<vector<double> &> it;
        size_t right_index;
        size_t left_index;
        double left_time;
        double right_time;
        double left_traj;
        double right_traj;
        double k;
        double value;
        double delta_x = 0.0;
        for (size_t i = 0; i < bm_t.size(); ++i) {
            time = bm_t[i] * (1.0 - tau) + tau;
            it = std::ranges::find_if(bm_t,
                                      [time](double v) { return v > time; });
            right_index = it == bm_t.end()
                              ? bm_t.size() - 1
                              : std::ranges::distance(bm_t.begin(), it);
            left_index = right_index - 1;
            left_time = bm_t[left_index];
            right_time = bm_t[right_index];
            left_traj = bm_traj[left_index];
            right_traj = bm_traj[right_index];
            k = (left_traj - right_traj) / (left_time - right_time);
            value = k * (time - left_time) + left_traj;
            delta_x += std::abs(value) * coe;
        }

        return Ok(delta_x);
    }
};
