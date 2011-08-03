
CFLAGS = -I${CIRCA_HOME}/src -rdynamic

# the "-undefined dynamic_lookup" flag is a Darwin thing

all:
	g++ ${CFLAGS} -shared -undefined dynamic_lookup sampledll.cpp -o sampledll.so
