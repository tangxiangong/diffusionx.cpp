# Examples CMakeLists.txt
cmake_minimum_required(VERSION 3.28)

# 如果作为独立项目构建
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(diffusionx_examples)
    
    # 查找 diffusionx 包
    find_package(diffusionx REQUIRED)
    set(DIFFUSIONX_TARGET diffusionx::diffusionx)
else()
    # 作为子项目构建
    set(DIFFUSIONX_TARGET diffusionx)
endif()

# 收集示例源文件
file(GLOB EXAMPLES_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 存储示例目标名称
set(EXAMPLE_TARGETS "")

# 为每个示例文件创建可执行程序
foreach(example_file ${EXAMPLES_SRC})
    get_filename_component(example_name ${example_file} NAME_WE)
    
    # 创建可执行程序
    add_executable(${example_name} ${example_file})
    
    # 添加到目标列表
    list(APPEND EXAMPLE_TARGETS ${example_name})
    
    # 链接 diffusionx 库
    target_link_libraries(${example_name} PRIVATE ${DIFFUSIONX_TARGET})
    
    # 设置 C++23 标准
    target_compile_features(${example_name} PRIVATE cxx_std_23)
    
    # 平台特定的编译器选项
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${example_name} PRIVATE
            -Wall -Wextra -Wpedantic
            $<$<CONFIG:Debug>:-g -O0>
            $<$<CONFIG:Release>:-O3 -DNDEBUG>
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${example_name} PRIVATE
            -Wall -Wextra -Wpedantic
            $<$<CONFIG:Debug>:-g -O0>
            $<$<CONFIG:Release>:-O3 -DNDEBUG>
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${example_name} PRIVATE
            /W4
            $<$<CONFIG:Debug>:/Od /Zi>
            $<$<CONFIG:Release>:/O2 /DNDEBUG>
        )
    endif()
    
    # 设置输出目录
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        # 独立构建时
        set_target_properties(${example_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
        )
    else()
        # 作为子项目构建时，使用父项目的输出目录
        set_target_properties(${example_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
        )
    endif()
    
    message(STATUS "Added example: ${example_name}")
endforeach()

# 添加一个自定义目标来构建所有示例
add_custom_target(examples
    DEPENDS ${EXAMPLE_TARGETS}
    COMMENT "Building all examples"
) 