
CFLAGS = -I${CIRCA_HOME}/src -rdynamic
LINKFLAGS = -lbox2d

MODULE_NAME = box2d

SOURCES = ${MODULE_NAME}.cpp

# the "-undefined dynamic_lookup" flag is a Darwin thing
all:
	g++ ${CFLAGS} -shared -undefined dynamic_lookup ${SOURCES} ${LINKFLAGS} -o ${MODULE_NAME}.so
