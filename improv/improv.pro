
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
