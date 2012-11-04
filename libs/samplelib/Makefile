
LIBNAME = samplelib

CFLAGS = -I../../include
LINKFLAGS = 

ifeq ($(shell uname), Darwin)
	# mac
	OS_FLAGS = -dynamiclib -undefined dynamic_lookup
else
	# linux
	OS_FLAGS = -fPIC -shared
endif

all:
	gcc ${CFLAGS} ${LINKFLAGS} ${OS_FLAGS} -ggdb ${LIBNAME}.cpp -o ${LIBNAME}.so
