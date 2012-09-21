

LIBNAME = cairo

CFLAGS = -I../../include -I/usr/local/Cellar/cairo/1.12.2/include
LINKFLAGS = -L/usr/local/Cellar/cairo/1.12.2/lib -lcairo

all:
	gcc ${CFLAGS} ${LINKFLAGS} -rdynamic -undefined dynamic_lookup -ggdb cairo.cpp -o cairo.so
