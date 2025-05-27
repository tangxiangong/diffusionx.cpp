/**
 * @file random.cppm
 * @brief Main module for random number generation and probability distributions
 * 
 * This module provides a comprehensive collection of random number generators
 * and probability distributions for stochastic simulations. It includes:
 * - Utility functions for random number generation
 * - Uniform, normal, exponential, gamma, Poisson, and stable distributions
 * - Thread-safe parallel generation capabilities
 * - Modern C++23 module interface
 */
export module diffusionx.random;

export import diffusionx.random.utils;
export import diffusionx.random.uniform;
export import diffusionx.random.exponential;
export import diffusionx.random.normal;
export import diffusionx.random.gamma;
export import diffusionx.random.poisson;
export import diffusionx.random.stable;