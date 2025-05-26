# DiffusionX 示例程序

本目录包含 DiffusionX 库的示例程序，展示如何使用各种随机数生成功能。

## 示例程序

- **`random.cpp`** - 基本的随机数生成示例，包括正态分布、指数分布和泊松分布
- **`test_modules.cpp`** - 模块系统测试示例，展示各种分布的使用

## 构建方式

### 1. 作为主项目的一部分构建

从项目根目录：
```bash
cmake -B build -G Ninja
cmake --build build
```

示例程序将输出到 `bin/` 目录。

### 2. 独立构建示例

如果 DiffusionX 已经安装到系统中，可以独立构建示例：

```bash
cd examples
cmake -B build -G Ninja
cmake --build build
```

示例程序将输出到 `examples/build/bin/` 目录。

### 3. 使用 CMake 目标

构建所有示例：
```bash
cmake --build build --target examples
```

构建特定示例：
```bash
cmake --build build --target random
cmake --build build --target test_modules
```

## 运行示例

### macOS/Linux
```bash
./bin/random
./bin/test_modules
```

### Windows
```cmd
.\bin\random.exe
.\bin\test_modules.exe
```

## 示例说明

### random.cpp
演示了基本的随机数生成功能：
- 生成正态分布随机数
- 生成指数分布随机数  
- 生成泊松分布随机数
- 错误处理示例

### test_modules.cpp
测试各种分布模块：
- 正态分布 (`randn`)
- 均匀分布 (`rand`)
- 指数分布 (`randexp`)
- 稳定分布 (`rand_stable`)

## 添加新示例

要添加新的示例程序：

1. 在 `examples/` 目录中创建新的 `.cpp` 文件
2. 重新运行 CMake 配置
3. 新的可执行程序将自动添加到构建系统中

示例模板：
```cpp
#include <iostream>
import diffusionx;

int main() {
    // 你的示例代码
    return 0;
}
``` 