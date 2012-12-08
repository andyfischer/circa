
-- This build file is leftover from something that once sorta worked. It doesn't
-- work now. But it might be a good place to start.

solution "Improv"
    configurations { "Debug", "Release" }
    language "C++"
    includedirs {"include"}
    flags { "Symbols" }
    targetdir "build"
    objdir "build/obj"

    configuration "Release"
        flags { "OptimizeSpeed" }

    configuration "Debug"
        defines { "DEBUG" }

    project "Improv"
        kind "WindowedApp"
        location "build"
        location "src"
        files {
            "src/*.cpp",
            }
        includedirs {".", "../include", "deps"}

        -- Freetype2
        buildoptions {"`freetype-config --cflags`"}
        linkoptions {"`freetype-config --libs~"}

        -- Cairo
        includedirs { "/usr/local/Cellar/cairo/1.12.2/include/cairo" }
        libdirs { "/usr/local/Cellar/cairo/1.12.2/lib" }
        links { "cairo" }

        -- Pango
        buildoptions {"`pkg-config --cflags pango`" }
        linkoptions {"`pkg-config --libs pango`" }
        links { "pangocairo-1.0.0" }

        libdirs {
            "../build"
        }

        configuration "Debug"
            links { "circa_d" }
            targetname "improv_d"

        configuration "Release"
            links { "circa" }
            targetname "improv"
