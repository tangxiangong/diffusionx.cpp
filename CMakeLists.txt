cmake_minimum_required(VERSION 3.28)

# vcpkg 工具链配置
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

# 如果 vcpkg_installed 目录存在，设置 CMAKE_PREFIX_PATH
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/arm64-osx")
endif()

project(diffusionx VERSION 0.1.0)

find_package(FFTW3 CONFIG REQUIRED)

# C++23 标准设置
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 启用 C++20 模块支持
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# macOS SDK 路径配置
if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE MACOS_SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot ${MACOS_SDK_PATH}")
endif()

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 收集所有模块文件
file(GLOB_RECURSE MODULE_FILES "${PROJECT_SOURCE_DIR}/src/*.cppm")

# 创建模块库
add_library(diffusionx_modules)
target_sources(diffusionx_modules
    PUBLIC
        FILE_SET CXX_MODULES FILES
        ${MODULE_FILES}
)

# 设置编译特性和选项
target_compile_features(diffusionx_modules PUBLIC cxx_std_23)
target_link_libraries(diffusionx_modules PUBLIC FFTW3::fftw3)

# 设置包含目录
target_include_directories(diffusionx_modules PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

# 主接口库
add_library(diffusionx INTERFACE)
target_link_libraries(diffusionx INTERFACE diffusionx_modules)

# 构建选项
option(BUILD_EXAMPLES "Build example programs" ON)

# 添加示例程序
if(BUILD_EXAMPLES AND EXISTS "${PROJECT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# 自动复制 compile_commands.json
add_custom_command(
    TARGET diffusionx_modules POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${PROJECT_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to project root"
)
