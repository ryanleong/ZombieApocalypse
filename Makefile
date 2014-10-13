DATE=$(shell date +"%F_%T")

all:
	$(MAKE) -C apocalypse all
	$(MAKE) -C visualise all
	$(MAKE) -C report all
	mkdir -p images
	mkdir -p output

clean:
	$(MAKE) -C apocalypse clean
	$(MAKE) -C visualise clean
	$(MAKE) -C report clean

localclean:

localclobber:
	rm -rf images/
	rm -rf output/

clobber: localclean localclobber
	$(MAKE) -C apocalypse clobber
	$(MAKE) -C visualise clobber
	$(MAKE) -C report clobber
	
backup: images out
	mv images images_$(DATE)
	mv output output_$(DATE)

png: images
	for f in images/*.img; do echo $$f; visualise/visualise $$f; done

dem: images
	for f in images/*.img; do echo $$f; visualise/demographics $$f; done

hist: images
	for f in images/*.dem; do echo $$f; visualise/histogram.py $${f%.dem}-hist.png < $$f; done

plot: out
	visualise/plot.py <out

.PHONY: all clean localclean clobber backup png plot
