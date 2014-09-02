SRC = apocalypse.c entity.c random.c simulation.c world.c
OBJS = $(SRC:%.c=%.o)

CC = gcc
CFLAGS = --std=gnu99 -O2 -g -Wall -fopenmp
LIBS = -lm -lgomp -lpng

all: dependencies apocalypse

apocalypse: $(OBJS)
	$(CC) $(CFLAGS) -o apocalypse $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f apocalypse dependencies cscope.out

dependencies:
	gcc $(CFLAGS) -MM $(SRC) > dependencies

tags:
	cscope -b

.PHONY: all clean clobber tags


include dependencies
