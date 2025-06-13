/**
 * @file tamsd.cppm
 * @brief Time-averaged mean squared displacement (TAMSD) computation
 * 
 * This module provides functions for computing the time-averaged mean squared displacement,
 * which is a key quantity for analyzing anomalous diffusion and ergodicity breaking.
 */

module;

#include <vector>

export module diffusionx.simulation.basic.tamsd;

import diffusionx.error;
import diffusionx.simulation.basic.utils;

using std::vector;

/**
 * @brief Computes the time-averaged mean squared displacement (TAMSD)
 * @param trajectory The trajectory data as a vector of positions
 * @param lag_time The lag time for computing the TAMSD
 * @return Result containing the TAMSD value, or an Error
 * 
 * The TAMSD is defined as:
 * δ²(Δ) = (1/(T-Δ)) ∫₀^(T-Δ) [x(t+Δ) - x(t)]² dt
 * 
 * For discrete trajectories, this becomes:
 * δ²(Δ) = (1/(N-Δ)) Σᵢ₌₀^(N-Δ-1) [x(i+Δ) - x(i)]²
 */
export Result<double> tamsd(const vector<double>& trajectory, size_t lag_time) {
    if (trajectory.empty()) {
        return Err(Error::InvalidArgument("Trajectory cannot be empty"));
    }
    
    if (lag_time == 0) {
        return Err(Error::InvalidArgument("Lag time must be positive"));
    }
    
    if (lag_time >= trajectory.size()) {
        return Err(Error::InvalidArgument("Lag time must be less than trajectory length"));
    }
    
    double sum = 0.0;
    size_t count = trajectory.size() - lag_time;
    
    for (size_t i = 0; i < count; ++i) {
        double displacement = trajectory[i + lag_time] - trajectory[i];
        sum += displacement * displacement;
    }
    
    return Ok(sum / static_cast<double>(count));
}

/**
 * @brief Computes the TAMSD for multiple lag times
 * @param trajectory The trajectory data as a vector of positions
 * @param max_lag_time The maximum lag time to compute
 * @return Result containing a vector of TAMSD values, or an Error
 */
export Result<vector<double>> tamsd_multiple(const vector<double>& trajectory, size_t max_lag_time) {
    if (trajectory.empty()) {
        return Err(Error::InvalidArgument("Trajectory cannot be empty"));
    }
    
    if (max_lag_time == 0) {
        return Err(Error::InvalidArgument("Maximum lag time must be positive"));
    }
    
    if (max_lag_time >= trajectory.size()) {
        return Err(Error::InvalidArgument("Maximum lag time must be less than trajectory length"));
    }
    
    vector<double> tamsd_values(max_lag_time);
    
    for (size_t lag = 1; lag <= max_lag_time; ++lag) {
        auto result = tamsd(trajectory, lag);
        if (!result.has_value()) {
            return Err(result.error());
        }
        tamsd_values[lag - 1] = result.value();
    }
    
    return Ok(std::move(tamsd_values));
}

/**
 * @brief Computes the ensemble-averaged TAMSD from multiple trajectories
 * @param trajectories Vector of trajectory data
 * @param lag_time The lag time for computing the TAMSD
 * @return Result containing the ensemble-averaged TAMSD value, or an Error
 */
export Result<double> ensemble_tamsd(const vector<vector<double>>& trajectories, size_t lag_time) {
    if (trajectories.empty()) {
        return Err(Error::InvalidArgument("Trajectories vector cannot be empty"));
    }
    
    double sum = 0.0;
    size_t valid_count = 0;
    
    for (const auto& trajectory : trajectories) {
        auto result = tamsd(trajectory, lag_time);
        if (result.has_value()) {
            sum += result.value();
            ++valid_count;
        }
    }
    
    if (valid_count == 0) {
        return Err(Error::InvalidArgument("No valid trajectories for TAMSD computation"));
    }
    
    return Ok(sum / static_cast<double>(valid_count));
}

/**
 * @brief Computes the TAMSD distribution from multiple trajectories
 * @param trajectories Vector of trajectory data
 * @param lag_time The lag time for computing the TAMSD
 * @return Result containing a vector of TAMSD values from each trajectory, or an Error
 */
export Result<vector<double>> tamsd_distribution(const vector<vector<double>>& trajectories, size_t lag_time) {
    if (trajectories.empty()) {
        return Err(Error::InvalidArgument("Trajectories vector cannot be empty"));
    }
    
    vector<double> tamsd_values;
    tamsd_values.reserve(trajectories.size());
    
    for (const auto& trajectory : trajectories) {
        auto result = tamsd(trajectory, lag_time);
        if (result.has_value()) {
            tamsd_values.push_back(result.value());
        }
    }
    
    if (tamsd_values.empty()) {
        return Err(Error::InvalidArgument("No valid trajectories for TAMSD computation"));
    }
    
    return Ok(std::move(tamsd_values));
}

/**
 * @brief Computes the ergodicity breaking parameter (EB)
 * @param trajectories Vector of trajectory data
 * @param lag_time The lag time for computing the TAMSD
 * @return Result containing the EB parameter, or an Error
 * 
 * The ergodicity breaking parameter is defined as:
 * EB = <δ²>² / <δ⁴> - 1
 * 
 * where <δ²> is the ensemble average of TAMSD and <δ⁴> is the ensemble average of TAMSD².
 * For ergodic processes, EB ≈ 0. For non-ergodic processes, EB > 0.
 */
export Result<double> ergodicity_breaking_parameter(const vector<vector<double>>& trajectories, size_t lag_time) {
    auto tamsd_dist_result = tamsd_distribution(trajectories, lag_time);
    if (!tamsd_dist_result.has_value()) {
        return Err(tamsd_dist_result.error());
    }
    
    auto tamsd_values = tamsd_dist_result.value();
    
    if (tamsd_values.size() < 2) {
        return Err(Error::InvalidArgument("Need at least 2 valid trajectories for EB computation"));
    }
    
    // Compute <δ²> and <δ⁴>
    double mean_tamsd = 0.0;
    double mean_tamsd_squared = 0.0;
    
    for (double tamsd_val : tamsd_values) {
        mean_tamsd += tamsd_val;
        mean_tamsd_squared += tamsd_val * tamsd_val;
    }
    
    mean_tamsd /= static_cast<double>(tamsd_values.size());
    mean_tamsd_squared /= static_cast<double>(tamsd_values.size());
    
    // Compute EB parameter
    if (mean_tamsd_squared == 0.0) {
        return Err(Error::InvalidArgument("Mean TAMSD squared is zero"));
    }
    
    double eb = ((mean_tamsd * mean_tamsd) / mean_tamsd_squared) - 1.0;
    
    return Ok(eb);
} 