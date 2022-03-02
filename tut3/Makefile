# using gcc :
COMPILER ?= $(GCC_PATH)gcc

#FLAGS ?= -O2 -Wall -Wno-variadic-macros -pedantic -g -DDEBUG -fopenmp $(GCC_SUPPFLAGS)
FLAGS ?= -O2 -Wall -Wno-variadic-macros -pedantic -g -fopenmp $(GCC_SUPPFLAGS)

LDFLAGS ?= -g -fopenmp
LDLIBS = -lm

EXECUTABLE = process_transactions

SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:src/%.c=obj/%.o)

all: release

release: $(OBJS)
	$(COMPILER) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDLIBS) 

obj/%.o: src/%.c | obj
	$(COMPILER) $(FLAGS) -o $@ -c $<

obj: 
	mkdir -p $@

clean:
	rm obj/*
	rm *.log
	rm ${EXECUTABLE} 
