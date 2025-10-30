cmake_minimum_required(VERSION 4.0)

project(diffusionx VERSION 0.1.0 LANGUAGES CXX)

FIND_PACKAGE(FFTW3 CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

if(APPLE)
    execute_process(COMMAND
        xcrun --show-sdk-path OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -isysroot ${MACOS_SDK_PATH}")
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

file(GLOB_RECURSE MODULE_FILES "${PROJECT_SOURCE_DIR}/src/*.cppm")

add_library(diffusionx SHARED)
target_sources(diffusionx
    PUBLIC
        FILE_SET CXX_MODULES FILES
        ${MODULE_FILES}
)
set_target_properties(diffusionx PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    CXX_SCAN_FOR_MODULES ON
)
target_link_libraries(diffusionx
    PRIVATE
    FFTW3::fftw3
)

option(BUILD_EXAMPLES "Build example programs" ON)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

add_custom_command(
    TARGET diffusionx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${PROJECT_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to project root"
)
