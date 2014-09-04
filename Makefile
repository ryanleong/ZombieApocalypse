SRC = apocalypse.c entity.c random.c simulation.c world.c
OBJS = $(SRC:%.c=%.o)

CC = gcc
CFLAGS = --std=gnu99 -O0 -g -Wall
LIBS = -lm -lgomp -lpng

all: dependencies apocalypse

apocalypse: $(OBJS)
	$(CC) $(CFLAGS) -o apocalypse $(OBJS) $(LIBS)
	mkdir images

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f apocalypse
	rm -f dependencies
	rm -f cscope.out
	rm -rf images/

dependencies:
	gcc $(CFLAGS) -MM $(SRC) > dependencies

tags:
	cscope -b

.PHONY: all clean clobber tags


include dependencies
