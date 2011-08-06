
CFLAGS = -I${CIRCA_HOME}/src -rdynamic -I/usr/local/Cellar/cairo/1.10.2/include
LINKFLAGS = -lbox2d -L/usr/local/Cellar/cairo/1.10.2/lib -lcairo

MODULE_NAME = cairo

SOURCES = ${MODULE_NAME}.cpp

# the "-undefined dynamic_lookup" flag is a Darwin thing
all:
	g++ ${CFLAGS} -shared -undefined dynamic_lookup ${SOURCES} ${LINKFLAGS} -o ${MODULE_NAME}.so
