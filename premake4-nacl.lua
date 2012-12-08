
solution "Circa"
    configurations { "Debug", "Release" }
    language "C++"
    includedirs {"include", "src" }
    flags { "Symbols" }
    targetdir "build/nacl"
    objdir "build/nacl/obj"
    location "build/nacl"

    defines { "CIRCA_DISABLE_DLL" }

    project "main"

    files {
        "src/*.cpp",
        "src/command_line/command_line.cpp",
        "src/generated/all_builtin_functions.cpp",
        "src/generated/all_builtin_types.cpp",
        "src/generated/setup_builtin_functions.cpp",
        "src/generated/stdlib_script_text.cpp",
        }
