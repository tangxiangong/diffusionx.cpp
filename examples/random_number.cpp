#include <print>

import diffusionx;

int main() {
    // 测试正态分布
    auto normal_result = randn<double>(10, 0.0, 1.0);
    if (normal_result) {
        std::println("正态分布样本生成成功，样本数量: {}", normal_result->size());
    } else {
        std::println("正态分布样本生成失败: {}", normal_result.error().message);
    }

    // 测试均匀分布
    auto uniform_result = rand<double>(5, 0.0, 1.0);
    if (uniform_result) {
        std::println("均匀分布样本生成成功，样本数量: {}", uniform_result->size());
    } else {
        std::println("均匀分布样本生成失败: {}", uniform_result.error().message);
    }

    // 测试指数分布
    auto exp_result = randexp<double>(3, 1.0);
    if (exp_result) {
        std::println("指数分布样本生成成功，样本数量: {}", exp_result->size());
    } else {
        std::println("指数分布样本生成失败: {}", exp_result.error().message);
    }

    // 测试稳定分布
    auto stable_result = rand_stable(1, 1.5, 0.0, 1.0, 0.0);
    if (stable_result) {
        std::println("稳定分布样本生成成功");
    } else {
        std::println("稳定分布样本生成失败: {}", stable_result.error().message);
    }

    return 0;
}
