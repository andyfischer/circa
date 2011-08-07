
CFLAGS = -I${CIRCA_HOME}/src -rdynamic -DOSX `sdl-config --cflags` `sdl-config --libs`

# the "-undefined dynamic_lookup" flag is a Darwin thing

SOURCES = sdl_img.cpp

all:
	g++ ${CFLAGS} -shared -undefined dynamic_lookup ${SOURCES} -o sdl_img.so
