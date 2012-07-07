
solution "Improv"
    configurations { "Debug", "Release" }
    language "C++"
    includedirs {"include"}
    flags { "Symbols" }

    configuration "Release"
        flags { "OptimizeSpeed" }

    configuration "Debug"
        defines { "DEBUG" }

    project "Improv"
        kind "WindowedApp"
        location "build"
        targetdir "build"
        files {
            "src/*.cpp",
            "engine/*.cpp",
            "build/*.cpp",
            }
        includedirs {".", "../include", "libs"}

        -- For freetype2 on OSX. Will need cross-platform fix
        includedirs {"/usr/X11/include", "/usr/X11/include/freetype2"}
        libdirs { "/usr/X11/lib" }
        links { "freetype" }

        -- Qt
        includedirs {
            "/usr/local/Cellar/qt/4.8.2/include",
            "/usr/local/Cellar/qt/4.8.2/Frameworks/QtCore.framework/Headers",
            "/usr/local/Cellar/qt/4.8.2/Frameworks/QtGui.framework/Headers",
            "/usr/local/Cellar/qt/4.8.2/Frameworks/QtOpenGL.framework/Headers",
        }

        linkoptions { "-F/usr/local/Cellar/qt/4.8.2/lib/"}
        libdirs {
            "../build"
        }
        links {
            "QtCore.framework",
            "QtGui.framework",
            "QtOpenGL.framework",
            "OpenGL.framework",
            "AGL.framework",
        }


 -- -F/usr/local/Cellar/qt/4.8.2/lib -L/usr/local/Cellar/qt/4.8.2/lib -L../build -L/usr/X11/lib -lfreetype -lcirca_d -framework OpenGL -framework AGL -framework QtOpenGL -L/usr/local/Cellar/qt/4.8.2/lib -F/usr/local/Cellar/qt/4.8.2/lib -framework QtGui -framework QtCore 

        configuration "Debug"
            links { "circa_d" }
            targetname "improv_d"

        configuration "Release"
            links { "circa_d" }
            targetname "improv"
