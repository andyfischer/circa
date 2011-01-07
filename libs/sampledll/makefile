
CFLAGS = -I${CIRCA_HOME}/src -rdynamic

all:
	g++ ${CFLAGS} -shared sampledll.cpp -o sampledll.so ${CIRCA_HOME}/build/libcirca_d.dylib
