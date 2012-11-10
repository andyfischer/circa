
LIBNAME = cairo

CFLAGS = -I../../include -I/usr/local/Cellar/cairo/1.12.2/include
LINKFLAGS = -L/usr/local/Cellar/cairo/1.12.2/lib -lcairo

ifeq ($(shell uname), Darwin)
	# mac
	OS_FLAGS = -dynamiclib -undefined dynamic_lookup
	LINKFLAGS += -framework OpenGL
else
	# linux
	OS_FLAGS = -fPIC -shared
endif

all:
	gcc ${CFLAGS} ${LINKFLAGS} ${OS_FLAGS} -ggdb ${LIBNAME}.cpp -o ${LIBNAME}.so
