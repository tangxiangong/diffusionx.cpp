build:
    rm -rf build && CC=/opt/homebrew/opt/llvm/bin/clang CXX=/opt/homebrew/opt/llvm/bin/clang++ cmake -B build -G Ninja && cmake --build build

release:
    rm -rf build && CC=/opt/homebrew/opt/llvm/bin/clang CXX=/opt/homebrew/opt/llvm/bin/clang++ cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build

debug:
    rm -rf build && CC=/opt/homebrew/opt/llvm/bin/clang CXX=/opt/homebrew/opt/llvm/bin/clang++ cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build

clean:
    rm -rf build

configure:
    CC=/opt/homebrew/opt/llvm/bin/clang CXX=/opt/homebrew/opt/llvm/bin/clang++ cmake -B build -G Ninja

# -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
