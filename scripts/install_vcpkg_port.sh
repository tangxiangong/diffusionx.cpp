#!/bin/bash

# DiffusionX vcpkg 端口安装脚本

set -e

echo "🚀 开始安装 DiffusionX vcpkg 端口..."

# 使用已知的 vcpkg 路径
VCPKG_ROOT="/Users/xiaoyu/vcpkg"

echo "📍 vcpkg 根目录: $VCPKG_ROOT"

# 检查 vcpkg 是否存在
if [ ! -d "$VCPKG_ROOT" ]; then
    echo "❌ 错误: 找不到 vcpkg 目录: $VCPKG_ROOT"
    echo "请确保 vcpkg 已正确安装"
    exit 1
fi

# 创建端口目录
PORT_DIR="$VCPKG_ROOT/ports/diffusionx"
echo "📁 创建端口目录: $PORT_DIR"
mkdir -p "$PORT_DIR"

# 复制端口文件
echo "📋 复制端口文件..."
cp vcpkg.json "$PORT_DIR/"
cp portfile.cmake "$PORT_DIR/"
cp usage "$PORT_DIR/"

echo "✅ 端口文件复制完成:"
echo "   - vcpkg.json"
echo "   - portfile.cmake"
echo "   - usage"

# 显示端口内容
echo ""
echo "📄 端口文件内容:"
echo "--- vcpkg.json ---"
cat "$PORT_DIR/vcpkg.json"
echo ""
echo "--- portfile.cmake (前10行) ---"
head -10 "$PORT_DIR/portfile.cmake"
echo "..."

echo ""
echo "🎯 下一步操作:"
echo "1. 在 GitHub 上创建 v0.1.0 标签"
echo "2. 更新 portfile.cmake 中的 SHA512 哈希值"
echo "3. 安装包: cd $VCPKG_ROOT && ./vcpkg install diffusionx"
echo "4. 运行测试: ./scripts/test_vcpkg_simple.sh"

echo ""
echo "✅ vcpkg 端口安装完成!" 