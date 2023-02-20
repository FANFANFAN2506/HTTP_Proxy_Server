TARGETS=request
CFLAGS=-std=c++11 -pedantic -Wall -Werror -ggdb3
OBJS = $(patsubst %,%.o,$(TARGETS)) request.o
all: $(TARGETS)
parse: request.o
	g++ $(CFLAGS) -g -o $@ $<
%.o:%.cpp
	g++ $(CFLAGS) -c $<

.PHONY:clean
clean:
	rm -f *~ $(TARGETS) $(OBJS)

request.o:request.hpp
