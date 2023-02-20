TARGETS=parse
CFLAGS=-std=c++11 -pedantic -Wall -Werror -ggdb3
OBJS = $(patsubst %,%.o,$(TARGETS)) parse.o
all: $(TARGETS)
parse: parse.o
	g++ $(CFLAGS) -g -o $@ $<
%.o:%.cpp
	g++ $(CFLAGS) -c $<

.PHONY:clean
clean:
	rm -f *~ $(TARGETS) $(OBJS)

parse.o:parse.hpp
