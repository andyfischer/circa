
CFLAGS = -L$(CIRCA_HOME)/build -lcirca_d -ggdb -Wall -DDEBUG -I$(CIRCA_HOME)/include

sample1: sample1.cpp
	g++ $(CFLAGS) -o sample1 sample1.cpp

reload_file: reload_file.c
	g++ $(CFLAGS) -o reload_file reload_file.c
