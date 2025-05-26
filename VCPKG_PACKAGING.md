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
wget https://github.com/your-username/diffusionx-cpp/archive/v0.1.0.tar.gz

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