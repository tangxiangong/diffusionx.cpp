cmake_minimum_required(VERSION 3.31)
project(diffusionx)

set(CMAKE_CXX_STANDARD 23)

# 设置库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_library(diffusionx INTERFACE)
target_include_directories(diffusionx INTERFACE
#    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# 自动为examples目录下每个cpp生成可执行文件
file(GLOB EXAMPLES_SRC "${PROJECT_SOURCE_DIR}/examples/*.cpp")
foreach (example_file ${EXAMPLES_SRC})
    get_filename_component(example_name ${example_file} NAME_WE)
    add_executable(${example_name} ${example_file})
    target_link_libraries(${example_name} PRIVATE diffusionx)
endforeach ()
