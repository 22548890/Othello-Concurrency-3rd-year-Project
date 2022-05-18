#COMPILER ?= $(GCC_PATH)gcc
# using mpirun :
COMPILER ?= mpicc

FLAGS ?= -O2 -Wall -Wno-variadic-macros -pedantic $(GCC_SUPPFLAGS)

LDFLAGS ?= -g
LDLIBS =

EXECUTABLE = mpi_sort 

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
	rm -f obj/* *.log
	rm $(EXECUTABLE) 
