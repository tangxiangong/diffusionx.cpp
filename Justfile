default: debug

build_dir := "build"
lib_dir := "lib"
bin_dir := "bin"

clean:
    @if [ -d "{{build_dir}}" ]; then rm -rf {{build_dir}}; fi
    @if [ -d "{{lib_dir}}" ]; then rm -rf {{lib_dir}}; fi
    @if [ -d "{{bin_dir}}" ]; then rm -rf {{bin_dir}}; fi

configure: install
    @cmake --preset=vcpkg

debug: clean configure
    @cmake --build {{build_dir}}

release: clean install
    @cmake --preset=vcpkg -DCMAKE_BUILD_TYPE=Release
    @cmake --build {{build_dir}}

install:
    @vcpkg install
