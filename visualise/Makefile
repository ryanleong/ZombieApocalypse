SRC = visualise.c demographics.c globalise.c
OBJS = $(SRC:%.c=%.o)

CC = gcc

CFLAGS = --std=gnu99 -O2 -g -Wall

LIBS = -lpng

all: dependencies visualise demographics globalise

visualise: visualise.o
	$(CC) $(CFLAGS) -o visualise visualise.o $(LIBS)

demographics: demographics.o
	$(CC) $(CFLAGS) -o demographics demographics.o $(LIBS)
	
globalise: globalise.o
	$(CC) $(CFLAGS) -o globalise globalise.o $(LIBS)

clean: 
	rm -f $(OBJS)

clobber: clean
	rm -f visualise
	rm -f demographics
	rm -f globalise
	rm -f dependencies
	rm -f cscope.out

dependencies: $(SRC)
	$(CC) $(CFLAGS) -MM $(SRC) > dependencies

tags:
	cscope -b

.PHONY: all clean clobber tags


include dependencies
