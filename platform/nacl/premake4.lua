
premake.gcc.cc = "pnacl-clang"
premake.gcc.cxx = "pnacl-clang++"
premake.gcc.ar = "pnacl-ar"

solution "Circa"
    configurations { "Debug", "Release" }
    language "C++"
    flags { "Symbols" }
    objdir "../../build/nacl"
    targetname "circa_cl.pexe"

    defines { "CIRCA_NACL", "CIRCA_DISABLE_DLL" }

    project "main"
        kind "ConsoleApp"

    includedirs {
        "../../include",
        "../../src",
        "$(NACL_SDK_ROOT)/include"
    }

    buildoptions {
			"-std=c++0x",
			"-U__STRICT_ANSI__",
			"-pthread",
			"-fno-stack-protector",
			"-fdiagnostics-show-option",
			"-Wunused-value",
			"-fdata-sections",
			"-ffunction-sections",
    }

    links {
        "nosys",
        "ppapi",
        "ppapi_cpp"
    }

    libdirs {
        "$(NACL_SDK_ROOT)/lib/mac_PNACL_pnacl/Debug"
    }

    linkoptions {
        -- "-Wl",
        -- "--gc-sections"
    }

    files {
        "../../src/*.cpp",
        "../../src/generated/all_builtin_functions.cpp",
        "../../src/generated/all_builtin_types.cpp",
        "../../src/generated/setup_builtin_functions.cpp",
        "../../src/generated/stdlib_script_text.cpp",
        "nacl_cl_main.cpp"
        }
