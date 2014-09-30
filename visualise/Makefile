SRC = visualise.c demographics.c
OBJS = $(SRC:%.c=%.o)

CC = gcc

CFLAGS = --std=gnu99 -O2 -g -Wall

LIBS = -lpng

all: dependencies visualise demographics

visualise: visualise.o
	$(CC) $(CFLAGS) -o visualise visualise.o $(LIBS)

demographics: demographics.o
	$(CC) $(CFLAGS) -o demographics demographics.o $(LIBS)

clean: 
	rm -f $(OBJS)

clobber: clean
	rm -f visualise
	rm -f demographics
	rm -f dependencies
	rm -f cscope.out

dependencies: $(SRC)
	$(CC) $(CFLAGS) -MM $(SRC) > dependencies

tags:
	cscope -b

.PHONY: all clean clobber tags


include dependencies
