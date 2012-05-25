
newoption {
    trigger = "windows",
    description = "Build for Windows platform"
}

solution "Circa"
    configurations { "Debug", "Release" }
    language "C++"
    includedirs {"include"}
    flags { "Symbols" }

    configuration "Release"
        flags { "OptimizeSpeed" }

    configuration "windows"
        defines { "WINDOWS" }

    project "static_lib"
        kind "StaticLib"
        targetname "circa"
        location "build"
        targetdir "build"
        files {
            "src/*.cpp",
            "src/generated/all_builtin_functions.cpp",
            "src/generated/all_builtin_types.cpp",
            "src/generated/setup_builtin_functions.cpp",
            "src/generated/stdlib_script_text.cpp",
            }
        includedirs {"src"}

        configuration "Debug"
            targetname "circa_d"

    project "command_line"
        kind "ConsoleApp"
        targetname "circa"
        targetdir "build"
        location "build"
        files {
            "src/tools/build_tool.cpp",
            "src/tools/command_line.cpp",
            "src/tools/debugger_repl.cpp",
            "src/tools/exporting_parser.cpp",
            "src/tools/file_checker.cpp",
            "src/tools/generate_cpp.cpp",
            "src/tools/repl.cpp"
        }
        links {"static_lib"}
        includedirs {"src"}

        configuration "Debug"
            targetname "circa_d"

    project "unit_tests"
        kind "ConsoleApp"
        targetname "circa_test"
        targetdir "build"
        location "build"
        files {"tests/internal/*.cpp"}
        includedirs {"src"}
        links {"static_lib"}
