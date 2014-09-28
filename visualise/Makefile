SRC = visualise.c
OBJS = $(SRC:%.c=%.o)

CC = gcc

CFLAGS = --std=gnu99 -O2 -g -Wall

LIBS = -lpng

all: dependencies visualise

visualise: $(OBJS)
	$(CC) $(CFLAGS) -o visualise $(OBJS) $(LIBS)

clean: 
	rm -f $(OBJS)

clobber: clean
	rm -f visualise
	rm -f dependencies
	rm -f cscope.out

dependencies: $(SRC)
	$(CC) $(CFLAGS) -MM $(SRC) > dependencies

tags:
	cscope -b

.PHONY: all clean clobber tags


include dependencies
