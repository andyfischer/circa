solution "Circa"
    configurations { "Debug", "Release" }
    language "C++"
    includedirs {"include"}
    flags { "Symbols" }

    configuration "Release"
        flags { "OptimizeSpeed" }

    project "static_lib"
        kind "StaticLib"
        targetname "circa"
        location "build"
        targetdir "build"
        files {
            "src/*.cpp",
            "src/tools/*.cpp",
            "src/generated/all_builtin_functions.cpp",
            "src/generated/all_builtin_types.cpp",
            "src/generated/setup_builtin_functions.cpp",
            "src/generated/stdlib_script_text.cpp",
            }
        excludes { "src/main.cpp" }
        includedirs {"include"}

        configuration "Debug"
            targetname "circa_d"

    project "command_line"
        kind "ConsoleApp"
        targetname "circa"
        targetdir "build"
        location "build"
        files {"src/main.cpp"}
        links {"static_lib"}

        configuration "Debug"
            targetname "circa_d"

    project "unit_tests"
        kind "ConsoleApp"
        targetname "circa_test"
        targetdir "build"
        location "build"
        files {"tests/internal/*.cpp"}
        links {"static_lib"}

