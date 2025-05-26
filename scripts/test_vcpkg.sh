#!/bin/bash

# DiffusionX vcpkg 集成测试脚本 (简化版)

set -e

echo "🚀 开始测试 DiffusionX vcpkg 集成..."

# 使用已知的 vcpkg 路径
VCPKG_ROOT="/Users/xiaoyu/vcpkg"
VCPKG_TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

echo "📍 vcpkg 根目录: $VCPKG_ROOT"
echo "🔧 工具链文件: $VCPKG_TOOLCHAIN"

# 检查工具链文件是否存在
if [ ! -f "$VCPKG_TOOLCHAIN" ]; then
    echo "❌ 错误: 找不到 vcpkg 工具链文件: $VCPKG_TOOLCHAIN"
    echo "请确保 vcpkg 已正确安装在 $VCPKG_ROOT"
    exit 1
fi

# 检查 diffusionx 包是否已安装
echo "🔍 检查 diffusionx 包是否已安装..."
if [ ! -d "$VCPKG_ROOT/installed/arm64-osx/lib" ] || [ ! -f "$VCPKG_ROOT/installed/arm64-osx/share/diffusionx/diffusionx-config.cmake" ]; then
    echo "⚠️  diffusionx 包未安装，请先运行:"
    echo "   cd $VCPKG_ROOT"
    echo "   ./vcpkg install diffusionx"
    echo ""
    echo "如果包不存在，请先将端口文件复制到 vcpkg 仓库:"
    echo "   mkdir -p $VCPKG_ROOT/ports/diffusionx"
    echo "   cp vcpkg.json portfile.cmake usage $VCPKG_ROOT/ports/diffusionx/"
    exit 1
fi

# 进入测试目录
cd test_vcpkg

# 清理之前的构建
rm -rf build

echo "🏗️  配置测试项目..."
cmake -B build \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" \
    -DCMAKE_BUILD_TYPE=Release \
    -DVCPKG_MANIFEST_MODE=OFF

echo "🔨 构建测试项目..."
cmake --build build

echo "🧪 运行测试..."
if [ -f "build/test_vcpkg" ]; then
    ./build/test_vcpkg
else
    echo "❌ 错误: 找不到测试可执行文件"
    ls -la build/
    exit 1
fi

echo "✅ vcpkg 集成测试完成!" 