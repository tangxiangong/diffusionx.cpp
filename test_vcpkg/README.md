# DiffusionX vcpkg 集成测试

这个目录包含一个独立的测试项目，用于验证 DiffusionX 库通过 vcpkg 的集成是否正常工作。

## 前提条件

1. 安装 vcpkg
2. 安装 DiffusionX 包：
   ```bash
   vcpkg install diffusionx
   ```

## 构建和运行

```bash
cd test_vcpkg

# 配置项目（使用 vcpkg 工具链）
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# 构建
cmake --build build

# 运行测试
./build/test_vcpkg  # Linux/macOS
# 或
.\build\Debug\test_vcpkg.exe  # Windows
```

## 预期输出

如果一切正常，你应该看到类似以下的输出：

```
Testing DiffusionX library via vcpkg...
✓ Normal distribution: generated 10 samples
  First 5 samples: -0.123 0.456 -0.789 0.234 -0.567
✓ Uniform distribution: generated 5 samples
  Samples: 0.123 0.456 0.789 0.234 0.567
✓ Exponential distribution: generated 3 samples
  Samples: 0.123 0.456 0.789
✓ Poisson distribution: generated 5 samples
  Samples: 2 1 3 2 1

🎉 All tests passed! DiffusionX library is working correctly via vcpkg.
```

## 故障排除

### 找不到 diffusionx 包
确保已经通过 vcpkg 安装了 diffusionx 包，并且在 cmake 配置时指定了正确的工具链文件。

### 编译器错误
确保使用支持 C++23 和 C++20 模块的编译器。

### 模块导入错误
检查编译器是否正确支持 C++20 模块系统。 