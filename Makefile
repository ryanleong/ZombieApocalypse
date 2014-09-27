SRC = apocalypse.c entity.c direction.c random.c simulation.c utils.c world.c
OBJS = $(SRC:%.c=%.o)

ifeq ($(HOSTNAME), avoca)
CC = mpicc
else
CC = gcc
endif

CFLAGS = --std=gnu99 -O2 -g -Wall -fopenmp

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

ifdef TIME
CFLAGS += -DTIME
endif

LIBS = -lm -lgomp

ifndef NIMAGES
LIBS += -lpng
endif

all: dependencies apocalypse

apocalypse: $(OBJS)
	$(CC) $(CFLAGS) -o apocalypse $(OBJS) $(LIBS)
	mkdir -p images

clean: cleanpdf
	rm -f $(OBJS)

clobber: clean clobberpdf
	rm -f apocalypse
	rm -f dependencies
	rm -f cscope.out
	rm -rf images/

dependencies: $(SRC)
	$(CC) $(CFLAGS) -MM $(SRC) > dependencies

tags:
	cscope -b

## Report ##

%.eps: %.dia
	dia -t eps -e $@ $<

%.pdf: %.svg
	inkscape -A $@ $<

%.pdf: %.dot
	dot -Tsvg -O $<
	inkscape -A $@ $<.svg
	rm $<.svg

pdf: tr.pdf

tr.pdf: tr.tex model.pdf movement.eps USGompertzCurve.pdf
	pdflatex tr.tex

cleanpdf:
	rm -f *.aux *.out *.log

clobberpdf: cleanpdf
	rm -f tr.pdf

.PHONY: all pdf clean cleanpdf clobber clobberpdf tags


include dependencies
