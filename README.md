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

- CMake 4.0+
- 支持 C++23 和 C++20 模块的编译器：
  - LLVM/Clang 20.1.5+ (推荐使用 Homebrew 版本)
  - GCC 14+ (实验性支持)
  - MSVC 2022+ (Windows)
- Ninja 构建系统
- vcpkg 包管理器
- just 命令运行器（可选，推荐）

## 构建

### 安装依赖

#### macOS (使用 Homebrew)

```bash
# 安装编译工具
brew install llvm ninja cmake just

# 安装 vcpkg
git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh

# 设置环境变量（添加到 ~/.zshrc 或 ~/.bashrc）
export VCPKG_ROOT=~/vcpkg
```

### 使用 just 构建（推荐）

```bash
# 构建 debug 版本（默认）
just

# 或者显式指定
just debug

# 构建 release 版本
just release

# 增量构建（不清理）
just build

# 只清理构建产物
just clean

# 安装依赖
just install

# 重新配置
just configure
```

### 手动构建

```bash
# 安装依赖
vcpkg install

# 配置项目
cmake --preset=vcpkg

# 构建
cmake --build build

# 安装库（可选）
cmake --install build --prefix /usr/local
# 或安装到自定义位置
cmake --install build --prefix ~/my-libs
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

构建完成后，示例程序位于 `bin/examples` 目录：

```bash
# 运行随机数生成示例
./bin/examples/random_number
```

## 在其他项目中使用

安装库后，可以在其他 CMake 项目中使用：

### CMakeLists.txt 配置

```cmake
cmake_minimum_required(VERSION 3.28)
project(my_project)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# 查找已安装的 diffusionx 库
find_package(diffusionx REQUIRED)

# 创建可执行文件
add_executable(my_app main.cpp)

# 链接 diffusionx 库
target_link_libraries(my_app PRIVATE diffusionx::diffusionx)
```

### 使用示例

```cpp
import diffusionx;
#include <print>

int main() {
    auto samples = randn<double>(1000, 0.0, 1.0);
    if (samples) {
        std::println("Generated {} samples", samples->size());
    }
    return 0;
}
```

### 构建项目

```bash
# 如果安装到了自定义位置，需要设置 CMAKE_PREFIX_PATH
cmake -B build -DCMAKE_PREFIX_PATH=~/my-libs
cmake --build build
```

## 技术细节

- 使用 C++20 模块系统实现模块化设计
- 支持并行随机数生成，自动利用多核处理器
- 使用 `std::expected` 进行现代错误处理
- 所有模块都编译为预编译模块 (.pcm) 文件，提高编译速度
- 生成共享库 `libdiffusionx.dylib` (macOS) / `libdiffusionx.so` (Linux)
- 使用 vcpkg 进行依赖管理
- 支持标准 CMake 安装，可集成到其他项目
- 提供完整的 CMake 包配置文件（`diffusionxConfig.cmake`）
- 安装内容包括：
  - 共享库文件
  - C++ 模块文件（.cppm）
  - CMake 配置文件
  - 版本信息文件

## 已验证平台

- ✅ macOS 14.5+ with Homebrew LLVM 21.1
- ⚠️ 其他平台需要相应的 C++20 模块支持

## 注意事项

- 确保使用支持 C++20 模块的编译器版本
- 在 macOS 上推荐使用 Homebrew 的 LLVM 而不是 Apple Clang
- 模块系统仍在快速发展中，不同编译器的支持程度可能有所不同
