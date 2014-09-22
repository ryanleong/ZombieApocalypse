SRC = apocalypse.c entity.c direction.c random.c simulation.c utils.c world.c
OBJS = $(SRC:%.c=%.o)

CC = gcc
CFLAGS = --std=gnu99 -O0 -g -Wall -fopenmp

ifdef NDEBUG
CFLAGS += -DNDEBUG
endif

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

LIBS = -lm -lgomp -lpng

all: dependencies apocalypse

apocalypse: $(OBJS)
	$(CC) $(CFLAGS) -o apocalypse $(OBJS) $(LIBS)
	mkdir -p images

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f apocalypse
	rm -f dependencies
	rm -f cscope.out
	rm -rf images/

dependencies: $(SRC)
	$(CC) $(CFLAGS) -MM $(SRC) > dependencies

tags:
	cscope -b

.PHONY: all clean clobber tags


include dependencies
