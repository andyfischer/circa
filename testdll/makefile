
CFLAGS = -I${CIRCA_HOME}/src -fvisibility=hidden -rdynamic
LIBS = ${CIRCA_HOME}/build/debug/libcirca.a

all:
	g++ ${CFLAGS} -shared td.cpp -o td.so ${LIBS}
