# 设置策略以允许空的 include 文件夹（因为这是一个基于模块的库）
set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO tangxiangong/diffusionx.cpp
    REF v0.1.0
    SHA512 eefaa7335e6233f955d33a905d300083ed9b20196365a880fe18776bc925d1f4aa5f970d4aaafdbe1bb414918c9b49ca56d218b477aed2381b8f90e1df63c23c
    HEAD_REF main
)

# 检查编译器对 C++20 模块的支持
if(VCPKG_TARGET_IS_WINDOWS)
    if(VCPKG_PLATFORM_TOOLSET MATCHES "v14[0-2]")
        message(FATAL_ERROR "diffusionx requires Visual Studio 2022 (v143) or later for C++20 modules support")
    endif()
elseif(VCPKG_TARGET_IS_OSX)
    # 在 macOS 上，检查是否有 Homebrew LLVM
    if(NOT EXISTS "/opt/homebrew/opt/llvm/bin/clang++")
        message(WARNING 
            "Homebrew LLVM not found. Apple Clang does not support C++20 modules. "
            "The library will be built with limited functionality. "
            "To install Homebrew LLVM: brew install llvm"
        )
        # 如果没有 Homebrew LLVM，移除编译器设置，使用系统默认编译器
        list(REMOVE_ITEM VCPKG_CMAKE_CONFIGURE_OPTIONS 
            "-DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++"
            "-DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang"
        )
    endif()
elseif(VCPKG_TARGET_IS_LINUX)
    # 对于 Linux，假设使用足够新的编译器
    # 实际部署时可能需要更严格的检查
endif()

# 配置 CMake 选项
set(VCPKG_CMAKE_CONFIGURE_OPTIONS)

# 如果启用了 examples 特性，则构建示例
if("examples" IN_LIST FEATURES)
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS -DBUILD_EXAMPLES=ON)
else()
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS -DBUILD_EXAMPLES=OFF)
endif()

# 配置 CMake
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${VCPKG_CMAKE_CONFIGURE_OPTIONS}
        -DCMAKE_CXX_STANDARD=23
        -DCMAKE_CXX_STANDARD_REQUIRED=ON
        -DCMAKE_CXX_EXTENSIONS=OFF
        -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++
        -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang
)

# 构建
vcpkg_cmake_build()

# 安装
vcpkg_cmake_install()

# 修复 CMake 配置文件
vcpkg_cmake_config_fixup(
    PACKAGE_NAME diffusionx
    CONFIG_PATH lib/cmake/diffusionx
)

# 删除调试版本的包含文件（如果存在）
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# 安装许可证文件
if(EXISTS "${SOURCE_PATH}/LICENSE")
    vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
elseif(EXISTS "${SOURCE_PATH}/LICENSE.txt")
    vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
else()
    # 如果没有找到许可证文件，创建一个占位符
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" "See project homepage for license information.")
endif()

# 复制使用说明
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}") 