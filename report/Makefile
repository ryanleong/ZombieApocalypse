all: tr.pdf

%.eps: %.dia
	dia -t eps -e $@ $<

%.pdf: %.svg
        inkscape -A $@ $<

%.pdf: %.dot
	dot -Tsvg -O $<
	inkscape -A $@ $<.svg
	rm $<.svg

tr.pdf: tr.tex model.pdf movement.eps USGompertzCurve.pdf
	pdflatex tr.tex

clean:
	rm -f *.aux *.out *.log

clobber: clean
	rm -f tr.pdf

.PHONY: all clean clobber

