SRC = apocalypse.c communication.c entity.c direction.c random.c simulation.c world.c
OBJS = $(SRC:%.c=%.o)

CFLAGS = --std=gnu99 -O2 -g -Wall -fopenmp

ifdef NIMAGES
CFLAGS += -DNIMAGES
endif

ifdef NPOPULATION
CFLAGS += -DNPOPULATION
endif

ifdef NDETAILED_STATS
CFLAGS += -DNDETAILED_STATS
endif

ifdef NCUMULATIVE_STATS
CFLAGS += -DNCUMULATIVE_STATS
endif

ifdef OUTPUT_EVERY
CFLAGS += -DOUTPUT_EVERY=$(OUTPUT_EVERY)
endif

ifdef IMAGES_EVERY
CFLAGS += -DIMAGES_EVERY=$(IMAGES_EVERY)
endif

ifdef POPULATION_EVERY
CFLAGS += -DPOPULATION_EVERY=$(POPULATION_EVERY)
endif

ifndef NTIME
CFLAGS += -DTIME
endif

ifdef USE_MPI
CFLAGS += -DUSE_MPI
endif

ifeq ($(HOSTNAME), avoca)
CC = mpicc
else
CC = gcc
endif

ifdef USE_MPI
CC = mpicc
endif

LIBS = -lm -lgomp

all: dependencies apocalypse

apocalypse: $(OBJS)
	$(CC) $(CFLAGS) -o apocalypse $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f apocalypse
	rm -f dependencies
	rm -f cscope.out

dependencies: $(SRC)
	$(CC) $(CFLAGS) -MM $(SRC) > dependencies

tags:
	cscope -b

.PHONY: all clean clobber tags


include dependencies
