
INCLUDEPATH += ../src

HEADERS = src/ScriptEnv.h \
          src/Viewport.h \
          src/BackgroundScript.h \
          src/MouseState.h

SOURCES = src/main.cpp \
          src/ScriptEnv.cpp \
          src/Viewport.cpp

LIBS += -L../build -lcirca_d
CONFIG += debug

QT += opengl
