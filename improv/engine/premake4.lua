
solution "ImprovEngine"
    configurations { "Debug", "Release" }
    language "C++"
    includedirs {"include"}
    flags { "Symbols" }

    configuration "Release"
        flags { "OptimizeSpeed" }

    configuration "Debug"
        defines { "DEBUG" }

    project "ImprovEngine"
        kind "StaticLib"
        location "build"
        targetdir "build"
        files {
            "*.cpp",
            }
        includedirs {".", "../../include", "../libs"}

        -- For freetype2 on OSX. Will need cross-platform fix
        includedirs {"/usr/X11/include", "/usr/X11/include/freetype2"}

        configuration "Debug"
            targetname "circa_d"
