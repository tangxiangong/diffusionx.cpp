/**
 * @file csv.cppm
 * @brief CSV output utilities for trajectory data
 * 
 * This module provides utilities for exporting trajectory data to CSV format.
 */

module;

#include <vector>
#include <fstream>
#include <string>
#include <sstream>

export module diffusionx.simulation.basic.csv;

import diffusionx.error;
import diffusionx.simulation.basic.utils;

using std::vector;
using std::string;

/**
 * @brief Writes a single trajectory to a CSV file
 * @param filename The output filename
 * @param times Vector of time points
 * @param positions Vector of position values
 * @return Result indicating success or an Error
 */
export Result<int> write_trajectory_csv(
    const string& filename,
    const vector<double>& times,
    const vector<double>& positions
) {
    if (times.size() != positions.size()) {
        return Err(Error::InvalidArgument("Times and positions vectors must have the same size"));
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return Err(Error::IoError("Failed to open file for writing: " + filename));
    }
    
    // Write header
    file << "time,position\n";
    
    // Write data
    for (size_t i = 0; i < times.size(); ++i) {
        file << times[i] << "," << positions[i] << "\n";
    }
    
    file.close();
    return Ok(0);
}

/**
 * @brief Writes multiple trajectories to a CSV file
 * @param filename The output filename
 * @param trajectories Vector of trajectory pairs (times, positions)
 * @return Result indicating success or an Error
 */
export Result<int> write_multiple_trajectories_csv(
    const string& filename,
    const vector<vec_pair>& trajectories
) {
    if (trajectories.empty()) {
        return Err(Error::InvalidArgument("Trajectories vector cannot be empty"));
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return Err(Error::IoError("Failed to open file for writing: " + filename));
    }
    
    // Write header
    file << "trajectory_id,time,position\n";
    
    // Write data
    for (size_t traj_id = 0; traj_id < trajectories.size(); ++traj_id) {
        const auto& [times, positions] = trajectories[traj_id];
        
        if (times.size() != positions.size()) {
            return Err(Error::InvalidArgument("Times and positions vectors must have the same size for trajectory " + std::to_string(traj_id)));
        }
        
        for (size_t i = 0; i < times.size(); ++i) {
            file << traj_id << "," << times[i] << "," << positions[i] << "\n";
        }
    }
    
    file.close();
    return Ok(0);
}

/**
 * @brief Writes TAMSD data to a CSV file
 * @param filename The output filename
 * @param lag_times Vector of lag times
 * @param tamsd_values Vector of TAMSD values
 * @return Result indicating success or an Error
 */
export Result<int> write_tamsd_csv(
    const string& filename,
    const vector<double>& lag_times,
    const vector<double>& tamsd_values
) {
    if (lag_times.size() != tamsd_values.size()) {
        return Err(Error::InvalidArgument("Lag times and TAMSD values vectors must have the same size"));
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return Err(Error::IoError("Failed to open file for writing: " + filename));
    }
    
    // Write header
    file << "lag_time,tamsd\n";
    
    // Write data
    for (size_t i = 0; i < lag_times.size(); ++i) {
        file << lag_times[i] << "," << tamsd_values[i] << "\n";
    }
    
    file.close();
    return Ok(0);
}

/**
 * @brief Writes MSD data to a CSV file
 * @param filename The output filename
 * @param times Vector of time points
 * @param msd_values Vector of MSD values
 * @return Result indicating success or an Error
 */
export Result<int> write_msd_csv(
    const string& filename,
    const vector<double>& times,
    const vector<double>& msd_values
) {
    if (times.size() != msd_values.size()) {
        return Err(Error::InvalidArgument("Times and MSD values vectors must have the same size"));
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return Err(Error::IoError("Failed to open file for writing: " + filename));
    }
    
    // Write header
    file << "time,msd\n";
    
    // Write data
    for (size_t i = 0; i < times.size(); ++i) {
        file << times[i] << "," << msd_values[i] << "\n";
    }
    
    file.close();
    return Ok(0);
}

/**
 * @brief Reads a trajectory from a CSV file
 * @param filename The input filename
 * @return Result containing the trajectory data (times, positions), or an Error
 */
export Result<vec_pair> read_trajectory_csv(const string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return Err(Error::IoError("Failed to open file for reading: " + filename));
    }
    
    vector<double> times;
    vector<double> positions;
    string line;
    
    // Skip header
    if (!std::getline(file, line)) {
        return Err(Error::IoError("File is empty or cannot read header"));
    }
    
    // Read data
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        string time_str;
        string pos_str;
        
        if (!std::getline(iss, time_str, ',') || !std::getline(iss, pos_str)) {
            return Err(Error::IoError("Invalid CSV format"));
        }
        
        try {
            double time = std::stod(time_str);
            double position = std::stod(pos_str);
            times.push_back(time);
            positions.push_back(position);
        } catch (const std::exception&) {
            return Err(Error::IoError("Invalid number format in CSV"));
        }
    }
    
    file.close();
    
    if (times.empty()) {
        return Err(Error::IoError("No data found in CSV file"));
    }
    
    return Ok(std::make_pair(std::move(times), std::move(positions)));
} 