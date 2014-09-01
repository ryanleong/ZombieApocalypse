
SRC = simulation.c random.c world.c apocalypse.c
OBJS = $(SRC:%.c=%.o)

CC = gcc
CFLAGS = --std=gnu99 -O2 -g -Wall
LIBS = -lm

all: apocalypse

apocalypse: $(OBJS)
	$(CC) $(CFLAGS) -o apocalypse $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)

clobber:	clean
	rm -f apocalypse

depend:
	gcc $(CFLAGS) -MM $(SRC) > Dependencies

tags:
	cscope -b

.PHONY:		all clean clobber depend tags


include Dependencies
