cmake_minimum_required(VERSION 3.28)
project(diffusionx)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 自动生成 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 平台检测
message(STATUS "Building for ${CMAKE_SYSTEM_NAME} on ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# 检查编译器对模块的支持
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15.0")
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
    message(STATUS "C++20 modules enabled for Clang ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11.0")
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
    message(STATUS "C++20 modules enabled for GCC ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.29")
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
    message(STATUS "C++20 modules enabled for MSVC ${CMAKE_CXX_COMPILER_VERSION}")
else()
    message(WARNING "C++20 modules may not be fully supported by ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
endif()

# 跨平台编译器配置
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(APPLE)
        # macOS 特定配置
        execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE MACOS_SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot ${MACOS_SDK_PATH}")
    elseif(UNIX)
        # Linux 特定配置
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC 特定配置
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC 特定配置
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++23")
endif()

# 设置库输出目录（跨平台）
# 如果不是通过 vcpkg 构建，使用项目本地目录
if(NOT VCPKG_TOOLCHAIN)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()

# Windows 特定配置
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    # 为不同配置类型设置输出目录（仅在非 vcpkg 构建时）
    if(NOT VCPKG_TOOLCHAIN)
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/bin)
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/lib)
            set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/lib)
        endforeach()
    endif()
endif()

# 收集模块文件
file(GLOB_RECURSE MODULE_FILES "${PROJECT_SOURCE_DIR}/src/*.cppm")

# 创建模块库
add_library(diffusionx_modules)

# 根据模块支持情况配置源文件
if(CMAKE_CXX_SCAN_FOR_MODULES)
    target_sources(diffusionx_modules
        PUBLIC
            FILE_SET CXX_MODULES FILES
            ${MODULE_FILES}
    )
    message(STATUS "Using C++20 modules for ${CMAKE_SYSTEM_NAME}")
else()
    # 如果不支持模块，可以添加传统的头文件实现
    message(STATUS "Falling back to traditional headers for ${CMAKE_SYSTEM_NAME}")
    # 这里可以添加传统的 .cpp 文件作为备选
endif()

# 设置模块包含目录
target_include_directories(diffusionx_modules PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# 设置编译特性
target_compile_features(diffusionx_modules PUBLIC cxx_std_23)

# 平台特定的编译器选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(diffusionx_modules PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(diffusionx_modules PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(diffusionx_modules PRIVATE
        /W4
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
    )
endif()

# 设置模块属性
if(CMAKE_CXX_SCAN_FOR_MODULES)
    set_target_properties(diffusionx_modules PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

# 主接口库
add_library(diffusionx INTERFACE)
target_link_libraries(diffusionx INTERFACE diffusionx_modules)

# 添加构建选项
option(BUILD_EXAMPLES "Build example programs" ON)

# 添加示例程序子目录
if(BUILD_EXAMPLES AND EXISTS "${PROJECT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# 自动复制 compile_commands.json 到项目根目录（跨平台，仅在非 vcpkg 构建时）
if(CMAKE_EXPORT_COMPILE_COMMANDS AND NOT VCPKG_TOOLCHAIN)
    add_custom_command(
        TARGET diffusionx_modules POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${PROJECT_SOURCE_DIR}/compile_commands.json"
        COMMENT "Copying compile_commands.json to project root"
        VERBATIM
    )
endif()

# 安装配置（可选）
include(GNUInstallDirs)

install(TARGETS diffusionx diffusionx_modules
    EXPORT diffusionx-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_LIBDIR}/cxx/miu
)

# 安装头文件
if(EXISTS "${PROJECT_SOURCE_DIR}/include")
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

# 导出目标配置
install(EXPORT diffusionx-targets
    FILE diffusionx-targets.cmake
    NAMESPACE diffusionx::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/diffusionx
)

# 创建配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/diffusionx-config-version.cmake"
    VERSION 0.1.0
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/diffusionx-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/diffusionx-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/diffusionx
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/diffusionx-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/diffusionx-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/diffusionx
)
