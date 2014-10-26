all: tr.pdf slides.pdf

DIAGRAMS = model.pdf movement.eps border1.eps border2.eps toolchain.pdf mesh.eps
GRAPHS = USGompertzCurve.pdf fertilisation_probability.pdf

SIZE_500 = step-000000.png step-003650.png step-018250.png step-053100.png step-100000.png step-000000.pdf step-003650.pdf step-018250.pdf step-053100.pdf step-100000.pdf
SIZE_500_POPULATIONS = equal_birth.pdf stable_1000000.pdf stable.pdf uncontrolled_birth.pdf density_birth.pdf

TORUS_PNG = torus/step-002190-torus.png torus/step-006935-torus.png torus/step-016790-torus.png torus/step-042340-torus.png torus/step-059130-torus.png \
	torus/step-084315-torus.png torus/step-091615-torus.png torus/step-129575-torus.png torus/step-462820-torus.png torus/step-588745-torus.png
TORUS_HIST = step-002190.pdf step-006935.pdf step-016790.pdf step-042340.pdf step-059130.pdf \
	step-084315.pdf step-091615.pdf step-129575.pdf step-462820.pdf step-588745.pdf

SCALING = scaling-128.eps scaling-256.eps scaling-512.eps scaling-1024.eps scaling-2048.eps scaling-4096.eps scaling-8192.eps scaling-16384.eps

FIGURES = torus/title.png torus/planet.pdf $(DIAGRAMS) $(GRAPHS) $(SIZE_500_POPULATIONS) $(SIZE_500) $(TORUS_PNG) $(TORUS_HIST) $(SCALING)

SLIDES_FIGURES = $(GRAPHS) $(SIZE_500_POPULATIONS) $(DIAGRAMS) torus/planet.pdf $(TORUS_PNG) $(TORUS_HIST)

%.eps: %.dia
	dia -t eps -e $@ $<

%.pdf: %.svg
	inkscape -A $@ $<

%.pdf: %.dot
	dot -Tsvg -O $<
	inkscape -A $@ $<.svg
	rm $<.svg

%.pdf: model/%.population
	../visualise/plot.py $@ < $<

%.pdf: model/%.img
	../visualise/demographics $< $<.dem
	../visualise/histogram.py $@ < $<.dem
	rm $<.dem

%.pdf: torus/%.dem
	../visualise/histogram.py $@ < $<

%.png: model/%.img
	../visualise/visualise $< $@

scaling-%.eps: ../testing/single_times ../testing/strong_scaling.m
	../testing/strong_scaling.m $< `echo $@ | grep -o [0-9]*` $@

tr.pdf: tr.tex references.bib $(FIGURES)
	pdflatex tr.tex
	bibtex tr.aux
	pdflatex tr.tex
	pdflatex tr.tex

slides.pdf: slides.tex $(SLIDES_FIGURES)
	pdflatex slides.tex
	pdflatex slides.tex

fertilisation_probability.pdf: fertilisation_probability.py
	./fertilisation_probability.py $@

clean:
	rm -f *.aux *.out *.log *.bbl  *.blg  *.dvi *.toc

clobber: clean
	rm -f *.pdf *.png *-converted-to.pdf *.eps

.PHONY: all clean clobber

