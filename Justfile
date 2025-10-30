default: debug

build_dir := "build"
lib_dir := "lib"
bin_dir := "bin"

debug:
    @if [ -d "{{build_dir}}" ]; then rm -rf {{build_dir}}; fi
    @if [ -d "{{lib_dir}}" ]; then rm -rf {{lib_dir}}; fi
    @if [ -d "{{bin_dir}}" ]; then rm -rf {{bin_dir}}; fi
    @cmake -B {{build_dir}} -G Ninja
    @cmake --build {{build_dir}}

release:
    @if [ -d "{{build_dir}}" ]; then rm -rf {{build_dir}}; fi
    @if [ -d "{{lib_dir}}" ]; then rm -rf {{lib_dir}}; fi
    @if [ -d "{{bin_dir}}" ]; then rm -rf {{bin_dir}}; fi
    @cmake -B {{build_dir}} -G Ninja -DCMAKE_BUILD_TYPE=Release
    @cmake --build {{build_dir}}
