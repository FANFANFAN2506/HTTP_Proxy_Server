TARGETS=main
CFLAGS=-std=c++11 -pedantic -Wall -Werror -ggdb3 -pthread -lpthread
OBJS = $(patsubst %,%.o,$(TARGETS)) proxy.o 
all: $(TARGETS)
main: main.o proxy.o
	g++ -g $(CFLAGS)  -o $@ $^
main.o:main.cpp
	g++ $(CFLAGS) -c $<
proxy.o:proxy.cpp
	g++ $(CFLAGS) -c $<

.PHONY:clean
clean:
	rm -f *.gch *~ $(TARGETS) $(OBJS)
