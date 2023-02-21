TARGETS=request
CFLAGS=-std=c++11 -pedantic -Wall -Werror -ggdb3
OBJS = $(patsubst %,%.o,$(TARGETS)) request.o utils.o
all: $(TARGETS)
request: request.o utils.o
	g++ $(CFLAGS) -g -o $@ $<
%.o:%.cpp
	g++ $(CFLAGS) -c $<

.PHONY:clean
clean:
	rm -f *~ $(TARGETS) $(OBJS)

utils.o:utils.hpp
