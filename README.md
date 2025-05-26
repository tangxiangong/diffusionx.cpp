# DiffusionX C++ 库

这是一个使用 C++20 模块系统的随机数生成库，支持多种概率分布。

## 特性

- 使用 C++23 标准和 C++20 模块系统
- 支持多种概率分布：
  - 正态分布 (Normal)
  - 均匀分布 (Uniform)
  - 指数分布 (Exponential)
  - 伽马分布 (Gamma)
  - 泊松分布 (Poisson)
  - 稳定分布 (Stable)
- 并行随机数生成
- 现代 C++ 错误处理 (std::expected)

## 构建要求

- CMake 3.28+
- 支持 C++23 和 C++20 模块的编译器：
  - LLVM/Clang 20.1.5+ (推荐使用 Homebrew 版本)
  - GCC 14+ (实验性支持)
  - MSVC 2022+ (Windows)
- Ninja 构建系统

## 构建

### macOS (使用 Homebrew LLVM)

```bash
# 安装依赖
brew install llvm ninja cmake

# 构建
mkdir build
cd build
CXX=/opt/homebrew/opt/llvm/bin/clang++ CC=/opt/homebrew/opt/llvm/bin/clang cmake -G Ninja ..
ninja
```

### 其他平台

```bash
mkdir build
cd build
cmake -G Ninja ..
ninja
```

## 模块结构

- `diffusionx.error` - 错误处理类型
- `diffusionx.random.utils` - 随机数生成工具
- `diffusionx.random.uniform` - 均匀分布
- `diffusionx.random.normal` - 正态分布
- `diffusionx.random.exponential` - 指数分布
- `diffusionx.random.gamma` - 伽马分布
- `diffusionx.random.poisson` - 泊松分布
- `diffusionx.random.stable` - 稳定分布
- `diffusionx` - 主模块，导出所有子模块

## 使用示例

```cpp
#include <print>
import diffusionx;

int main() {
    // 生成正态分布样本
    auto result = randn<double>(1000, 0.0, 1.0);
    if (result) {
        std::println("生成了 {} 个正态分布样本", result->size());
        for (const auto& val : *result) {
            std::println("{}", val);
        }
    } else {
        std::println("错误: {}", result.error().message);
    }
    
    // 生成均匀分布样本
    auto uniform = rand<double>(100, 0.0, 1.0);
    if (uniform) {
        std::println("生成了 {} 个均匀分布样本", uniform->size());
    }
    
    return 0;
}
```

## 运行示例

构建完成后，可以运行示例程序：

```bash
# 运行随机数生成示例
./bin/random

# 运行模块测试示例
./bin/test_modules
```

## 技术细节

- 使用 C++20 模块系统实现模块化设计
- 支持并行随机数生成，自动利用多核处理器
- 使用 `std::expected` 进行现代错误处理
- 所有模块都编译为预编译模块 (.pcm) 文件，提高编译速度
- 生成静态库 `libdiffusionx_modules.a` 供链接使用

## 已验证平台

- ✅ macOS 14.5+ with Homebrew LLVM 20.1.5
- ⚠️ 其他平台需要相应的 C++20 模块支持

## 注意事项

- 确保使用支持 C++20 模块的编译器版本
- 在 macOS 上推荐使用 Homebrew 的 LLVM 而不是 Apple Clang
- 模块系统仍在快速发展中，不同编译器的支持程度可能有所不同 