# DiffusionX 示例程序

本目录包含 DiffusionX 库的示例程序，展示如何使用各种随机数生成功能。

## 示例程序

- **`random_number.cpp`** - 基本的随机数生成示例，包括正态分布、指数分布和泊松分布

## 构建方式

### 使用 just 构建（推荐）

从项目根目录：
```bash
# 构建 debug 版本（默认）
just

# 构建 release 版本
just release

# 增量构建（不清理）
just build
```

示例程序将输出到 `bin/examples` 目录。

### 手动构建

```bash
# 安装依赖
vcpkg install

# 配置项目
cmake --preset=vcpkg

# 构建
cmake --build build
```

## 运行示例

```bash
# macOS/Linux
./bin/examples/random_number

# Windows
.\bin\examples\random_number.exe
```
