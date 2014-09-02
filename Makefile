
CFLAGS=--std=gnu99 -O2 -g -Wall # -fopenmp
LDFLAGS=-lm -lgomp

all: apocalypse

apocalypse: apocalypse.o random.o simulation.o world.o bitarray.o entity.o

apocalypse.o: world.h entity.h random.h simulation.h

simulation.o: simulation.c simulation.h common.h bitarray.h

random.o: random.c random.h clock.h

world.o: world.c world.h clock.h entity.h

entity.o: entity.c entity.h clock.h common.h constants.h

bitarray.o: bitarray.c bitarray.h

clean:
	rm -f *.o apocalypse
