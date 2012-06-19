
INCLUDEPATH += ../include

QT += opengl

HEADERS = \
          src/GLWidget.h \
          src/Window.h \
          src/Scripts.h

SOURCES = src/main.cpp \
          src/GLWidget.cpp \
          src/Window.cpp \
          src/Scripts.cpp \
          src/qt_bindings.cpp

LIBS += -L../build
LIBS += -lcirca_d

CONFIG += opengl release

MOC_DIR = build
OBJECTS_DIR = build

ICON = assets/improv.icns
