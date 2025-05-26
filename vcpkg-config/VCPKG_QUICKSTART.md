# DiffusionX vcpkg 快速开始指南

## 🚀 快速设置

### 1. 安装端口文件

```bash
# 运行自动化安装脚本
./scripts/install_vcpkg_port.sh
```

### 2. 创建 GitHub 发布版本

在 GitHub 上创建 `v0.1.0` 标签：

```bash
git tag v0.1.0
git push origin v0.1.0
```

### 3. 更新 SHA512 哈希值

```bash
# 下载源码包并计算哈希
wget https://github.com/tangxiangong/diffusionx.cpp/archive/v0.1.0.tar.gz
sha512sum v0.1.0.tar.gz

# 将输出的哈希值更新到 /Users/xiaoyu/vcpkg/ports/diffusionx/portfile.cmake
```

### 4. 安装包

```bash
cd /Users/xiaoyu/vcpkg
./vcpkg install diffusionx
```

### 5. 测试安装

```bash
# 返回项目目录
cd /Users/xiaoyu/Projects/diffusionx-cpp

# 运行测试
./scripts/test_vcpkg_simple.sh
```

## 📋 完整流程示例

```bash
# 1. 设置端口
./scripts/install_vcpkg_port.sh

# 2. 创建 GitHub 标签（在 GitHub 网页上操作或使用 git）
git tag v0.1.0
git push origin v0.1.0

# 3. 计算并更新哈希值
wget https://github.com/tangxiangong/diffusionx.cpp/archive/v0.1.0.tar.gz
HASH=$(sha512sum v0.1.0.tar.gz | cut -d' ' -f1)
echo "SHA512: $HASH"

# 手动编辑 /Users/xiaoyu/vcpkg/ports/diffusionx/portfile.cmake
# 将 SHA512 行替换为实际的哈希值

# 4. 安装包
cd /Users/xiaoyu/vcpkg
./vcpkg install diffusionx

# 5. 测试
cd /Users/xiaoyu/Projects/diffusionx-cpp
./scripts/test_vcpkg_simple.sh
```

## 🔧 故障排除

### 哈希值不匹配
如果遇到 SHA512 哈希值不匹配的错误：
1. 重新下载源码包
2. 重新计算哈希值
3. 更新 `portfile.cmake` 中的 SHA512 值

### 编译错误
确保使用支持 C++23 和 C++20 模块的编译器：
- macOS: 使用 Homebrew LLVM (`/opt/homebrew/opt/llvm/bin/clang++`)
- 确保编译器版本足够新

### 包未找到
检查端口文件是否正确复制到 vcpkg 目录：
```bash
ls -la /Users/xiaoyu/vcpkg/ports/diffusionx/
```

## 📖 使用已安装的包

在你的项目中使用 DiffusionX：

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.28)
project(my_project)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(diffusionx CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE diffusionx::diffusionx)
```

### main.cpp
```cpp
#include <print>
import diffusionx;

int main() {
    auto samples = randn<double>(1000, 0.0, 1.0);
    if (samples) {
        std::println("Generated {} normal samples", samples->size());
    }
    return 0;
}
```

### 构建
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/Users/xiaoyu/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
./build/my_app
``` 

# DiffusionX vcpkg 打包总结

## 📦 已创建的文件

为了将 DiffusionX 库打包为 vcpkg 端口，我已经创建了以下文件：

### 核心 vcpkg 文件
- `vcpkg.json` - 包清单文件，定义包的基本信息和依赖
- `portfile.cmake` - vcpkg 构建脚本，定义如何下载、构建和安装包
- `usage` - 使用说明文件，提供集成指导
- `LICENSE` - MIT 许可证文件

### 项目修改
- 修改了 `CMakeLists.txt`，添加了 vcpkg 支持：
  - 添加 `BUILD_EXAMPLES` 选项
  - 在 vcpkg 构建时禁用本地输出目录设置
  - 在 vcpkg 构建时禁用 compile_commands.json 复制

### 测试和文档
- `test_vcpkg/` - 独立的测试项目，验证 vcpkg 集成
- `scripts/test_vcpkg.sh` - 自动化测试脚本（通用版）
- `scripts/test_vcpkg_simple.sh` - 简化版测试脚本
- `scripts/install_vcpkg_port.sh` - 端口安装脚本
- `VCPKG_PACKAGING.md` - 详细的打包指南
- `VCPKG_SUMMARY.md` - 本总结文档

## 🚀 使用步骤

### 1. 创建 vcpkg 端口

```bash
# 在 vcpkg 仓库中创建端口目录
cd /path/to/vcpkg
mkdir -p ports/diffusionx

# 复制端口文件（或使用自动化脚本）
cp /path/to/diffusionx-cpp/vcpkg.json ports/diffusionx/
cp /path/to/diffusionx-cpp/portfile.cmake ports/diffusionx/
cp /path/to/diffusionx-cpp/usage ports/diffusionx/

# 或者使用自动化脚本
./scripts/install_vcpkg_port.sh
```

### 2. 更新 portfile.cmake

在 `ports/diffusionx/portfile.cmake` 中更新：
- `REPO`: 替换为实际的 GitHub 仓库地址
- `REF`: 替换为实际的版本标签
- `SHA512`: 替换为实际的源码包 SHA512 哈希值

### 3. 安装和测试

```bash
# 安装包
vcpkg install diffusionx

# 测试特性
vcpkg install diffusionx[examples]

# 运行集成测试
cd /path/to/diffusionx-cpp
./scripts/test_vcpkg_simple.sh  # 推荐使用简化版
```

## 📋 包特性

- **名称**: `diffusionx`
- **版本**: `0.1.0`
- **许可证**: MIT
- **支持平台**: 除 UWP 和 ARM 外的所有平台
- **特性**:
  - `examples`: 构建示例程序

## 🔧 编译器要求

- **C++23 标准支持**
- **C++20 模块支持**
- **Windows**: Visual Studio 2022 (MSVC 19.29+)
- **Linux**: Clang 15.0+ 或 GCC 11.0+
- **macOS**: Clang 15.0+ (推荐 Homebrew LLVM)

## 📖 使用示例

### CMakeLists.txt
```cmake
find_package(diffusionx CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE diffusionx::diffusionx)
```

### C++ 代码
```cpp
#include <print>
import diffusionx;

int main() {
    auto samples = randn<double>(1000, 0.0, 1.0);
    if (samples) {
        std::println("Generated {} samples", samples->size());
    }
    return 0;
}
```

## ⚠️ 注意事项

1. **模块支持**: 确保编译器完全支持 C++20 模块
2. **标准库**: 在 Linux 上使用 Clang 时推荐 libc++
3. **版本兼容**: 需要相对较新的编译器版本
4. **平台限制**: 当前不支持 UWP 和 ARM 平台

## 🔄 后续步骤

1. **发布版本**: 在 GitHub 上创建 v0.1.0 标签
2. **计算哈希**: 获取源码包的 SHA512 哈希值
3. **提交端口**: 向 vcpkg 仓库提交 Pull Request
4. **测试验证**: 在不同平台上测试端口

## 📚 相关文档

- `VCPKG_PACKAGING.md` - 详细的打包指南
- `test_vcpkg/README.md` - 测试项目说明
- `BUILD.md` - 构建指南
- `README.md` - 项目主文档 

# DiffusionX vcpkg 打包指南

本文档说明如何将 DiffusionX 库打包为 vcpkg 端口。

## 文件结构

为了将 DiffusionX 打包为 vcpkg 端口，需要以下文件：

```
ports/diffusionx/
├── vcpkg.json          # 包清单文件
├── portfile.cmake      # 构建脚本
└── usage               # 使用说明
```

## 创建端口

### 1. 在 vcpkg 仓库中创建端口目录

```bash
cd /path/to/vcpkg
mkdir -p ports/diffusionx
```

### 2. 复制端口文件

将项目根目录下的以下文件复制到 `ports/diffusionx/` 目录：
- `vcpkg.json`
- `portfile.cmake`
- `usage`

### 3. 更新 portfile.cmake

在 `portfile.cmake` 中更新以下信息：
- `REPO`: 替换为实际的 GitHub 仓库地址
- `REF`: 替换为实际的版本标签
- `SHA512`: 替换为实际的源码包 SHA512 哈希值

获取 SHA512 哈希值：
```bash
# 下载源码包
wget https://github.com/tangxiangong/diffusionx.cpp/archive/v0.1.0.tar.gz

# 计算 SHA512
sha512sum v0.1.0.tar.gz
```

### 4. 测试端口

```bash
# 安装端口
./vcpkg install diffusionx

# 测试特性
./vcpkg install diffusionx[examples]
```

## 使用方法

### 在项目中使用

1. 安装 diffusionx：
```bash
vcpkg install diffusionx
```

2. 在 CMakeLists.txt 中：
```cmake
find_package(diffusionx CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE diffusionx::diffusionx)
```

3. 在代码中：
```cpp
#include <print>
import diffusionx;

int main() {
    auto samples = randn<double>(1000, 0.0, 1.0);
    if (samples) {
        std::println("Generated {} samples", samples->size());
    }
    return 0;
}
```

## 编译器要求

- **Windows**: Visual Studio 2022 (MSVC 19.29+)
- **Linux**: Clang 15.0+ 或 GCC 11.0+
- **macOS**: Clang 15.0+ (推荐使用 Homebrew LLVM)

## 特性

- `examples`: 构建示例程序

## 注意事项

1. **C++20 模块支持**: 确保编译器支持 C++20 模块
2. **C++23 标准**: 库需要 C++23 标准支持
3. **平台限制**: 当前不支持 UWP 和 ARM 平台

## 故障排除

### 模块编译错误
确保使用支持 C++20 模块的编译器版本。

### 链接错误
检查编译器和标准库的兼容性。

### 平台不支持
检查 `vcpkg.json` 中的 `supports` 字段。

## 贡献

如果需要更新端口或修复问题，请：

1. 更新相应的文件
2. 测试端口构建
3. 提交 Pull Request 到 vcpkg 仓库 
