TARGETS=request
CFLAGS=-std=c++11 -pedantic -Wall -Werror -ggdb3
OBJS = $(patsubst %,%.o,$(TARGETS))
all: $(TARGETS)
request:request.o
	g++ $(CFLAGS) -g -o $@ $<
%.o:%.cpp
	g++ $(CFLAGS) -c $<

.PHONY:clean
clean:
	rm -f *~ $(TARGETS) $(OBJS)

