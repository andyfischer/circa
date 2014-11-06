
solution "Improv"
    configurations { "Debug", "Release" }
    language "C++"
    includedirs {"../include"}
    flags { "Symbols" }
    targetdir "build"
    objdir "build/obj"
    defines { "NACL" }

    configuration "Release"
        flags { "OptimizeSpeed" }

    configuration "Debug"
        defines { "DEBUG" }

    project "Improv"
        kind "ConsoleApp"
        location "build"
        location "src"
        files {
            "src/*.cpp",
            }
        includedirs {".", "../include", "deps"}

        -- SDL
        buildoptions {"`sdl-config --cflags`"}
        linkoptions {"`sdl-config --libs`"}
        
        -- Freetype2
        buildoptions {"`freetype-config --cflags`"}
        linkoptions {"`freetype-config --libs`"}

        linkoptions {
            "-framework OpenGL"
        }

        libdirs {
            "../build"
        }

        configuration "Debug"
            links { "circa_d" }
            targetname "improv_d"

        configuration "Release"
            links { "circa" }
            targetname "improv"
