/**
 * @file diffusionx.cppm
 * @brief Main module for the DiffusionX C++ library
 * 
 * DiffusionX is a modern C++23 library for stochastic process simulation and analysis.
 * It provides:
 * - Comprehensive random number generation and probability distributions
 * - Stochastic process simulation framework
 * - Statistical analysis tools
 * - Error handling utilities
 * - Thread-safe parallel computation
 * 
 * This is the main entry point that exports all core functionality.
 * 
 * @version 0.1.0
 * @author Xiangong Tang
 */
export module diffusionx;

export import diffusionx.error;
export import diffusionx.random;
export import diffusionx.simulation;
