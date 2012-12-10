
premake.gcc.cc = "pnacl-clang"
premake.gcc.cxx = "pnacl-clang++"
premake.gcc.ar = "pnacl-ar"

solution "Circa"
    configurations { "Debug", "Release" }
    language "C++"
    flags { "Symbols" }
    objdir "../../build/nacl"

    defines { "CIRCA_DISABLE_DLL" }

    project "main"
        kind "ConsoleApp"

    includedirs {
        "../../include",
        "../../src"
    }

    files {
        "../../src/*.cpp",
        "../../src/generated/all_builtin_functions.cpp",
        "../../src/generated/all_builtin_types.cpp",
        "../../src/generated/setup_builtin_functions.cpp",
        "../../src/generated/stdlib_script_text.cpp",
        "nacl_cl_main.cpp"
        }
