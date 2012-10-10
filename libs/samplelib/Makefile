
LIBNAME = samplelib

CFLAGS = -I../../include
LINKFLAGS = 

MAC_FLAGS = -rdynamic -undefined dynamic_lookup
LINUX_FLAGS = -fPIC -shared

all:
	gcc ${CFLAGS} ${LINKFLAGS} ${LINUX_FLAGS} -ggdb ${LIBNAME}.cpp -o ${LIBNAME}.so
