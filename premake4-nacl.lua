
if not os.getenv("PNACL_TOOLCHAIN") then 
  print("Error: PNACL_TOOLCHAIN envvar was not set. It should look something like this:")
  print("  /nacl_sdk/pepper24/toolchain/mac_x86_pnacl")
end

premake.gcc.cc = "$(PNACL_TOOLCHAIN)/newlib/bin/pnacl-clang"
premake.gcc.cxx = "$(PNACL_TOOLCHAIN)/newlib/bin/pnacl-clang++"
premake.gcc.ar = "$(PNACL_TOOLCHAIN)/newlib/bin/pnacl-ar"

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
        kind "ConsoleApp"

    files {
        "src/*.cpp",
        "src/generated/all_builtin_functions.cpp",
        "src/generated/all_builtin_types.cpp",
        "src/generated/setup_builtin_functions.cpp",
        "src/generated/stdlib_script_text.cpp",
        }
