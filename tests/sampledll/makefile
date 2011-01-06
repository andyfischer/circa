
CFLAGS = -I${CIRCA_HOME}/src -rdynamic
LIBS = ${CIRCA_HOME}/build/debug/libcirca.a

all:
	g++ ${CFLAGS} -shared sampledll.cpp -o sampledll.so ${LIBS}
