all: tr.pdf

FIGURES = model.pdf movement.eps USGompertzCurve.pdf equal_birth.pdf  stable_1000000.pdf  stable.pdf  uncontrolled_birth.pdf density_birth.pdf fertilisation_probability.pdf \
	step-000000.png  step-003650.png  step-018250.png  step-053100.png  step-100000.png step-000000.pdf  step-003650.pdf  step-018250.pdf  step-053100.pdf  step-100000.pdf

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

%.png: model/%.img
	../visualise/visualise $< $@

tr.pdf: tr.tex references.bib $(FIGURES)
	pdflatex tr.tex
	bibtex tr.aux
	pdflatex tr.tex
	pdflatex tr.tex

fertilisation_probability.pdf: fertilisation_probability.py
	./fertilisation_probability.py $@

clean:
	rm -f *.aux *.out *.log *.bbl  *.blg  *.dvi

clobber: clean
	rm -f *.pdf *.png

.PHONY: all clean clobber

