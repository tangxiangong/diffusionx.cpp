set_languages("c++23")

target("diffusionx")
    set_kind("shared")
    add_files("src/**.cppm")
    set_targetdir("lib")