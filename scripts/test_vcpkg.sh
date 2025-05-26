#!/bin/bash

# DiffusionX vcpkg 集成测试脚本

set -e

echo "🚀 开始测试 DiffusionX vcpkg 集成..."

# 检查 vcpkg 是否可用
if ! command -v vcpkg &> /dev/null; then
    echo "❌ 错误: 找不到 vcpkg 命令"
    echo "请确保 vcpkg 已安装并添加到 PATH 中"
    exit 1
fi

# 获取 vcpkg 根目录
VCPKG_ROOT=$(vcpkg integrate install 2>&1 | grep "CMake projects should use" | sed 's/.*: //' | sed 's/scripts.*//')
if [ -z "$VCPKG_ROOT" ]; then
    echo "❌ 错误: 无法确定 vcpkg 根目录"
    exit 1
fi

VCPKG_TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

echo "📍 vcpkg 根目录: $VCPKG_ROOT"
echo "🔧 工具链文件: $VCPKG_TOOLCHAIN"

# 检查工具链文件是否存在
if [ ! -f "$VCPKG_TOOLCHAIN" ]; then
    echo "❌ 错误: 找不到 vcpkg 工具链文件: $VCPKG_TOOLCHAIN"
    exit 1
fi

# 进入测试目录
cd test_vcpkg

echo "🏗️  配置测试项目..."
cmake -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" \
    -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
    -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang

echo "🔨 构建测试项目..."
cmake --build build

echo "🧪 运行测试..."
if [ -f "build/test_vcpkg" ]; then
    ./build/test_vcpkg
elif [ -f "build/Debug/test_vcpkg.exe" ]; then
    ./build/Debug/test_vcpkg.exe
else
    echo "❌ 错误: 找不到测试可执行文件"
    exit 1
fi

echo "✅ vcpkg 集成测试完成!" 