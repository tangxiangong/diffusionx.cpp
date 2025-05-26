#include <iostream>
#include <vector>

import diffusionx;

int main() {
    // 测试正态分布
    auto normal_result = randn<double>(10, 0.0, 1.0);
    if (normal_result) {
        std::cout << "正态分布样本生成成功，样本数量: " << normal_result->size() << std::endl;
    } else {
        std::cout << "正态分布样本生成失败: " << normal_result.error().message << std::endl;
    }

    // 测试均匀分布
    auto uniform_result = rand<double>(5, 0.0, 1.0);
    if (uniform_result) {
        std::cout << "均匀分布样本生成成功，样本数量: " << uniform_result->size() << std::endl;
    } else {
        std::cout << "均匀分布样本生成失败: " << uniform_result.error().message << std::endl;
    }

    // 测试指数分布
    auto exp_result = randexp<double>(3, 1.0);
    if (exp_result) {
        std::cout << "指数分布样本生成成功，样本数量: " << exp_result->size() << std::endl;
    } else {
        std::cout << "指数分布样本生成失败: " << exp_result.error().message << std::endl;
    }

    // 测试稳定分布
    auto stable_result = rand_stable(1, 1.5, 0.0, 1.0, 0.0);
    if (stable_result) {
        std::cout << "稳定分布样本生成成功" << std::endl;
    } else {
        std::cout << "稳定分布样本生成失败: " << stable_result.error().message << std::endl;
    }

    return 0;
} 