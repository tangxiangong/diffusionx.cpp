build:
    rm -rf build && mkdir build && cd build && cmake .. && cmake --build .

# -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
