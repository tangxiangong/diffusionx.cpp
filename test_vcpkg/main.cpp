#include <print>
#include <iostream>

import diffusionx.random;

int main() {
    std::println("Testing DiffusionX library via vcpkg...");
    
    // 测试正态分布
    auto normal_samples = randn<double>(10, 0.0, 1.0);
    if (normal_samples) {
        std::println("✓ Normal distribution: generated {} samples", normal_samples->size());
        std::print("  First 5 samples: ");
        for (size_t i = 0; i < std::min(5UL, normal_samples->size()); ++i) {
            std::print("{:.3f} ", (*normal_samples)[i]);
        }
        std::println("");
    } else {
        std::println("✗ Normal distribution failed: {}", normal_samples.error().message);
        return 1;
    }
    
    // 测试均匀分布
    auto uniform_samples = rand<double>(5, 0.0, 1.0);
    if (uniform_samples) {
        std::println("✓ Uniform distribution: generated {} samples", uniform_samples->size());
        std::print("  Samples: ");
        for (const auto& sample : *uniform_samples) {
            std::print("{:.3f} ", sample);
        }
        std::println("");
    } else {
        std::println("✗ Uniform distribution failed: {}", uniform_samples.error().message);
        return 1;
    }
    
    // 测试指数分布
    auto exp_samples = randexp<double>(3, 1.0);
    if (exp_samples) {
        std::println("✓ Exponential distribution: generated {} samples", exp_samples->size());
        std::print("  Samples: ");
        for (const auto& sample : *exp_samples) {
            std::print("{:.3f} ", sample);
        }
        std::println("");
    } else {
        std::println("✗ Exponential distribution failed: {}", exp_samples.error().message);
        return 1;
    }
    
    // 测试泊松分布
    auto poisson_samples = rand_poisson<unsigned int>(5, 2.0);
    if (poisson_samples) {
        std::println("✓ Poisson distribution: generated {} samples", poisson_samples->size());
        std::print("  Samples: ");
        for (const auto& sample : *poisson_samples) {
            std::print("{} ", sample);
        }
        std::println("");
    } else {
        std::println("✗ Poisson distribution failed: {}", poisson_samples.error().message);
        return 1;
    }
    
    std::println("\n🎉 All tests passed! DiffusionX library is working correctly via vcpkg.");
    return 0;
} 