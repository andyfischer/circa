
CFLAGS = -I${CIRCA_HOME}/src -rdynamic -DOSX
MODULE_NAME = imagemanip

SOURCES = ${MODULE_NAME}.cpp

# the "-undefined dynamic_lookup" flag is a Darwin thing
all:
	g++ ${CFLAGS} -shared -undefined dynamic_lookup ${SOURCES} ${LINKFLAGS} -o ${MODULE_NAME}.so
