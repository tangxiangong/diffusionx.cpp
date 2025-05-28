# Examples CMakeLists.txt
cmake_minimum_required(VERSION 3.28)

set(DIFFUSIONX_TARGET diffusionx)

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
    
    # 设置输出目录
    set_target_properties(${example_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    )
    
    message(STATUS "Added example: ${example_name}")
endforeach()

# 添加一个自定义目标来构建所有示例
add_custom_target(examples
    DEPENDS ${EXAMPLE_TARGETS}
    COMMENT "Building all examples"
) 