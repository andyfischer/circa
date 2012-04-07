
INCLUDEPATH += ../include

QT += opengl

HEADERS = \
          src/GLWidget.h \
          src/Window.h \
          src/ScriptCenter.h

SOURCES = src/main.cpp \
          src/GLWidget.cpp \
          src/Window.cpp \
          src/ScriptCenter.cpp \
          src/qt_bindings.cpp

LIBS += -L../build -lcirca_d
CONFIG += opengl

MOC_DIR = build
OBJECTS_DIR = build
