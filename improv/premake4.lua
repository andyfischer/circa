
solution "Improv"
    configurations { "Debug", "Release" }
    language "C++"
    includedirs {"../include"}
    flags { "Symbols" }
    targetdir "build"
    objdir "build/obj"

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
        includedirs {".", "../include", "deps", "/usr/local/include"}

        -- SDL
        buildoptions {"`sdl2-config --cflags`"}
        linkoptions {"`sdl2-config --libs`"}
        
        -- Freetype2
        buildoptions {"`freetype-config --cflags`"}
        linkoptions {"`freetype-config --libs`"}

        linkoptions {
            "-framework OpenGL"
        }

        libdirs {
            "../build",
            "/usr/local/lib"
        }

        links {
            "cairo"
        }

        configuration "Debug"
            links { "circa_d" }
            targetname "improv_d"

        configuration "Release"
            links { "circa" }
            targetname "improv"
