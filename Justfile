build:
    rm -rf build && mkdir build && cd build && cmake .. && cmake --build .

release:
    rm -rf build && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .

# -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
