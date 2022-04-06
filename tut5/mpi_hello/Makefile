#COMPILER ?= $(GCC_PATH)gcc
# using mpirun :
COMPILER ?= mpicc

FLAGS ?= -O2 -Wall -Wno-variadic-macros -pedantic $(GCC_SUPPFLAGS)

LDFLAGS ?= -g 
LDLIBS =

EXECUTABLE = mpi_hello 

SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:src/%.c=obj/%.o)

all: $(OBJS)
	$(COMPILER) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDLIBS) 

obj/%.o: src/%.c | obj
	$(COMPILER) $(FLAGS) -o $@ -c $<

obj:
	mkdir -p $@ 

clean:
	rm -f obj/*
	rm -f *.log
	rm ${EXECUTABLE} 
