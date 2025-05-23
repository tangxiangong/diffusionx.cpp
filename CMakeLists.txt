cmake_minimum_required(VERSION 3.31)
project(diffusionx)

set(CMAKE_CXX_STANDARD 23)

# 设置库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

file(GLOB_RECURSE ALL_SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

file(GLOB_RECURSE ALL_INCLUDE_FILES
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/include/*.h"
)

add_library(diffusionx_static STATIC ${ALL_SRC_FILES})
target_include_directories(diffusionx_static PUBLIC ${PROJECT_SOURCE_DIR}/include)
set_target_properties(diffusionx_static PROPERTIES PUBLIC_HEADER "${ALL_INCLUDE_FILES}")


add_library(diffusionx_shared SHARED ${ALL_SRC_FILES})
target_include_directories(diffusionx_shared PUBLIC ${PROJECT_SOURCE_DIR}/include)
set_target_properties(diffusionx_shared PROPERTIES PUBLIC_HEADER "${ALL_INCLUDE_FILES}")


add_library(diffusionx INTERFACE)
target_link_libraries(diffusionx INTERFACE diffusionx_shared)

file(GLOB EXAMPLES_SRC "${PROJECT_SOURCE_DIR}/examples/*.cpp")
foreach (example_file ${EXAMPLES_SRC})
    get_filename_component(example_name ${example_file} NAME_WE)
    add_executable(${example_name} ${example_file})
    target_link_libraries(${example_name} PRIVATE diffusionx)
endforeach ()
