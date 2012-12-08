
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
        includedirs {".", "../include", "libs"}

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
            links { "circa" }
            targetname "improv"
