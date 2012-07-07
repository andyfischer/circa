
INCLUDEPATH += ../include
INCLUDEPATH += .
INCLUDEPATH += libs

QT += opengl

HEADERS = \
          src/GLWidget.h \
          src/Window.h \
          src/Scripts.h \
          engine/Common.h \
          engine/FontBitmap.h \
          engine/Line.h \
          engine/RenderCommand.h \
          engine/RenderData.h \
          engine/RenderList.h \
          engine/ResourceManager.h \
          engine/ShaderUtils.h \
          engine/Sprite.h \
          engine/TextSprite.h \
          engine/TextTexture.h \
          engine/TextVbo.h \
          engine/Texture.h \


SOURCES = src/main.cpp \
          src/GLWidget.cpp \
          src/Window.cpp \
          src/Scripts.cpp \
          engine/Common.cpp \
          engine/FontBitmap.cpp \
          engine/Line.cpp \
          engine/RenderList.cpp \
          engine/ResourceManager.cpp \
          engine/ShaderUtils.cpp \
          engine/Sprite.cpp \
          engine/TextSprite.cpp \
          engine/TextTexture.cpp \
          engine/TextVbo.cpp \
          engine/Texture.cpp \

LIBS += -L../build

# For freetype2 on OSX. Will need cross-platform fix
LIBS += -L/usr/X11/lib -lfreetype
INCLUDEPATH += /usr/X11/include /usr/X11/include/freetype2

CONFIG += opengl

MOC_DIR = build
OBJECTS_DIR = build

ICON = assets/improv.icns

CONFIG += debug_and_release

CONFIG(debug, debug|release) {
  LIBS += -lcirca_d
  TARGET = improv_d
} else {
  LIBS += -lcirca
  TARGET = improv
}
