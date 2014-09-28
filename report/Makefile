all: tr.pdf

FIGURES = model.pdf movement.eps USGompertzCurve.pdf equal_birth_8000.pdf  stable_1000000.pdf  stable.pdf  uncontrolled_birth_8000.pdf

%.eps: %.dia
	dia -t eps -e $@ $<

%.pdf: %.svg
	inkscape -A $@ $<

%.pdf: %.dot
	dot -Tsvg -O $<
	inkscape -A $@ $<.svg
	rm $<.svg

%.pdf: %.population
	../visualise/plot.py $@ < $<

tr.pdf: tr.tex references.bib $(FIGURES)
	pdflatex tr.tex
	bibtex tr.aux
	pdflatex tr.tex
	pdflatex tr.tex

clean:
	rm -f *.aux *.out *.log

clobber: clean
	rm -f tr.pdf

.PHONY: all clean clobber

