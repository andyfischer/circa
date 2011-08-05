
CFLAGS = -I${CIRCA_HOME}/src -rdynamic

MODULE_NAME = osc

SOURCES = ${MODULE_NAME}.cpp

# the "-undefined dynamic_lookup" flag is a Darwin thing
all:
	g++ ${CFLAGS} -shared -undefined dynamic_lookup ${SOURCES} -o ${MODULE_NAME}.so
