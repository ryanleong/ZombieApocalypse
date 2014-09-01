
CFLAGS=--std=gnu99 -O2 -g -Wall
LDFLAGS=-lm

all: apocalypse

apocalypse: apocalypse.o random.o simulation.o world.o clock.h

simulation.o: simulation.c simulation.h

random.o: random.c random.h clock.h

world.o: world.c world.h clock.h agent.h

clean:
	rm -f *.o apocalypse
