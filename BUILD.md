# 跨平台构建指南

本项目支持在多个平台上构建，包括 macOS、Linux 和 Windows。

## 系统要求

### 通用要求
- CMake 3.28 或更高版本
- C++23 兼容的编译器
- Ninja 构建系统（推荐）

### 编译器要求

#### macOS
- **推荐**: LLVM Clang 15.0+ 
  ```bash
  brew install llvm ninja
  ```
- **备选**: Apple Clang（可能不支持所有 C++20 模块特性）

#### Linux
- **推荐**: Clang 15.0+ 或 GCC 11.0+
  ```bash
  # Ubuntu/Debian
  sudo apt install clang-15 libc++-15-dev libc++abi-15-dev ninja-build
  
  # 或者使用 GCC
  sudo apt install gcc-11 g++-11 ninja-build
  ```

#### Windows
- **推荐**: Visual Studio 2022 (MSVC 19.29+)
- **备选**: Clang-cl 或 MinGW-w64

## 构建步骤

### 1. 克隆项目
```bash
git clone <repository-url>
cd diffusionx-cpp
```

### 2. 配置构建

#### macOS (LLVM Clang)
```bash
CC=/opt/homebrew/opt/llvm/bin/clang \
CXX=/opt/homebrew/opt/llvm/bin/clang++ \
cmake -B build -G Ninja
```

#### Linux (Clang)
```bash
CC=clang-15 CXX=clang++-15 cmake -B build -G Ninja
```

#### Linux (GCC)
```bash
CC=gcc-11 CXX=g++-11 cmake -B build -G Ninja
```

#### Windows (Visual Studio)
```cmd
cmake -B build -G "Visual Studio 17 2022"
```

### 3. 构建项目
```bash
cmake --build build
```

### 4. 运行示例
```bash
# macOS/Linux
./bin/random
./bin/test_modules

# Windows
.\bin\random.exe
.\bin\test_modules.exe
```

### 5. 构建特定示例
```bash
# 构建所有示例
cmake --build build --target examples

# 构建特定示例
cmake --build build --target random
cmake --build build --target test_modules
```

## 构建配置

### Debug 构建
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Release 构建
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 安装

### 系统安装
```bash
cmake --build build --target install
```

### 自定义安装路径
```bash
cmake -B build -G Ninja -DCMAKE_INSTALL_PREFIX=/custom/path
cmake --build build --target install
```

## 故障排除

### C++20 模块支持
- **CMake 3.28+** 已正式支持 C++20 模块，无需设置实验性标志
- **`import std`** 仍然是实验性功能（CMake 3.30+），需要单独启用
- 如果编译器不支持 C++20 模块，CMake 会自动回退到传统的头文件模式

### 编译器版本检查
构建时会显示平台和编译器信息：
```
-- Building for Darwin on arm64
-- Compiler: Clang 20.1.5
-- C++20 modules enabled for Clang 20.1.5
```

### 常见问题

1. **模块不被识别**: 确保使用支持 C++20 模块的编译器版本
2. **链接错误**: 检查编译器和标准库的兼容性
3. **路径问题**: 使用 Ninja 生成器而不是 Make

## 平台特定说明

### macOS
- 使用 Homebrew 安装的 LLVM 获得最佳模块支持
- Apple Clang 可能不支持所有 C++20 特性

### Linux
- 推荐使用 libc++ 而不是 libstdc++（对于 Clang）
- 确保安装了正确版本的标准库开发包

### Windows
- Visual Studio 2022 提供最佳的 C++20 模块支持
- 使用 vcpkg 管理依赖项（如果需要）

## 高级功能

### 启用 `import std` (实验性)

如果你想使用 C++23 的 `import std` 功能，需要在 `project()` 调用之前设置：

```cmake
# 启用 import std 支持 (CMake 3.30+)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(your_project LANGUAGES CXX)

# 告诉 CMake 使用 import std
set(CMAKE_CXX_MODULE_STD 1)
```

**支持的编译器组合：**
- Clang 18.1.2+ with `-stdlib=libc++`
- MSVC 14.36+ (Visual Studio 17.6+)
- GCC 15+

## 集成到其他项目

### 使用 find_package
```cmake
find_package(diffusionx REQUIRED)
target_link_libraries(your_target PRIVATE diffusionx::diffusionx)
```

### 使用 FetchContent
```cmake
include(FetchContent)
FetchContent_Declare(
    diffusionx
    GIT_REPOSITORY <repository-url>
    GIT_TAG main
)
FetchContent_MakeAvailable(diffusionx)
target_link_libraries(your_target PRIVATE diffusionx::diffusionx)
``` 