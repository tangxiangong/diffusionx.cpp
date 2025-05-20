cmake_minimum_required(VERSION 3.31)
project(duffusionx)

set(CMAKE_CXX_STANDARD 23)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug Release RelWithDebInfo MinSizeRel")
endif()

# Set compiler flags based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Configuring for Release build")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Configuring for Debug build")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  message(STATUS "Configuring for Release with Debug Info build")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g -DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  message(STATUS "Configuring for Minimum Size Release build")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os -DNDEBUG")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

# Find TBB
find_package(TBB REQUIRED)

file(GLOB_RECURSE SRC "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 设置库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_library(duffusionx STATIC ${SRC})
target_link_libraries(duffusionx PUBLIC TBB::tbb)

add_library(duffusionx-shared SHARED ${SRC})
target_link_libraries(duffusionx-shared PUBLIC TBB::tbb)

# 自动为examples目录下每个cpp生成可执行文件
file(GLOB EXAMPLES_SRC "${PROJECT_SOURCE_DIR}/examples/*.cpp")
foreach(example_file ${EXAMPLES_SRC})
    get_filename_component(example_name ${example_file} NAME_WE)
    add_executable(${example_name} ${example_file})
    target_link_libraries(${example_name} PRIVATE duffusionx TBB::tbb)
endforeach()
