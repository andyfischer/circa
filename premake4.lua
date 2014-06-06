
solution "Circa"
    configurations { "Debug", "Release" }
    language "C++"
    flags { "Symbols" }
    targetdir "build"
    objdir "build/obj"
    includedirs { "include", "src", "3rdparty", "/usr/local/include" }
    libdirs { "/usr/local/lib" }

    configuration "Release"
        flags { "OptimizeSpeed" }

    configuration "Debug"
        defines { "DEBUG" }

    project "static_lib"
        kind "StaticLib"
        targetname "circa"
        location "src"
        files {
            "src/*.cpp",
            "src/ext/read_tar.cpp",
            "src/generated/stdlib_script_text.cpp",
            "3rdparty/tinymt/tinymt64.cc",
            "3rdparty/http-parser/http_parser.c"
            }

        configuration "Debug"
            targetname "circa_d"

    project "command_line"
        kind "ConsoleApp"
        targetname "circa"
        location "src"
        defines { "CIRCA_USE_LINENOISE" }
        files {
            "src/command_line/command_line.cpp",
            "src/command_line/command_line_main.cpp",
            "src/command_line/debugger_repl.cpp",
            "src/command_line/exporting_parser.cpp",
            "src/command_line/file_checker.cpp",
            "3rdparty/linenoise/linenoise.c",
        }
        links {"static_lib","dl"}

        configuration "Debug"
            targetname "circa_d"

    project "unit_tests"
        kind "ConsoleApp"
        targetname "circa_test"
        location "src"
        files {"src/unit_tests/*.cpp"}
        links {"static_lib","dl"}

        configuration "Release"
            targetname "circa_test_r"
