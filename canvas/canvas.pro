
INCLUDEPATH += ../include

HEADERS = \
          src/GLWidget.h \
          src/ScriptCenter.h

SOURCES = src/main.cpp \
          src/GLWidget.cpp \
          src/ScriptCenter.cpp \
          src/qt_bindings.cpp

LIBS += -L../build -lcirca_d
CONFIG += debug

MOC_DIR = build
OBJECTS_DIR = build
