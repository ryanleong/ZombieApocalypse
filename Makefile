SRC = apocalypse.c entity.c direction.c random.c simulation.c utils.c world.c
OBJS = $(SRC:%.c=%.o)

CC = gcc
CFLAGS = --std=gnu99 -O2 -g -Wall -fopenmp

ifeq ($(NDEBUG), 1)
CFLAGS += -DNDEBUG
endif

ifeq ($(NIMAGES), 1)
CFLAGS += -DNIMAGES
endif

ifeq ($(NPOPULATION), 1)
CFLAGS += -DNPOPULATION
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
