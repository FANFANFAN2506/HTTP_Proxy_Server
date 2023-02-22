TARGETS=request
CFLAGS=-std=c++11 -pedantic -Wall -Werror -ggdb3
OBJS = $(patsubst %,%.o,$(TARGETS)) response.o request.o
all: $(TARGETS)
request:request.o response.o
	g++ $(CFLAGS) -g -o $@ $<
%.o:%.cpp
	g++ $(CFLAGS) -c $<

.PHONY:clean
clean:
	rm -f *~ $(TARGETS) $(OBJS)

response.o:response.hpp
