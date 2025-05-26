cmake_minimum_required(VERSION 3.28)
project(test_diffusionx_vcpkg)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找 diffusionx 包
find_package(diffusionx CONFIG REQUIRED)

# 创建测试可执行程序
add_executable(test_vcpkg main.cpp)

# 链接 diffusionx 库
target_link_libraries(test_vcpkg PRIVATE diffusionx::diffusionx)

# 设置编译器特性
target_compile_features(test_vcpkg PRIVATE cxx_std_23)

# 平台特定的编译器选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(test_vcpkg PRIVATE
        -Wall -Wextra -Wpedantic
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_vcpkg PRIVATE
        -Wall -Wextra -Wpedantic
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(test_vcpkg PRIVATE
        /W4
    )
endif() 