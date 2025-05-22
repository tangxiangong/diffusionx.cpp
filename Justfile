build:
    rm -rf build && mkdir build && cd build && cmake .. && cmake --build . && cd ..


# -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake

