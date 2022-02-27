COMPILER ?= $(GCC_PATH)gcc
FLAGS ?= -O3 -Wall -fopenmp $(GCC_SUPPFLAGS)

LDFLAGS ?= -g -fopenmp
LDLIBS = -lm

EXECUTABLE = nth_tri

SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:src/%.c=obj/%.o)

all: $(OBJS)
	$(COMPILER) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDLIBS) 

obj/%.o: src/%.c | obj
	$(COMPILER) $(FLAGS) -o $@ -c $<

#create the obj directory if it does not exist
obj:
	mkdir -p $@

clean:
	rm -f obj/*
	rm *.log
	rm $(EXECUTABLE) 
