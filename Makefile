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
	
backup: images output
	mv images images_$(DATE)
	mv output output_$(DATE)

globalise: globalise-images globalise-output
	
globalise-images: images
	for f in images/step-??????-0-0.img; do echo $$f; visualise/globalise $$f; done

globalise-output: output
	visualise/globalise output/apocalypse-0-0.out

png: images images/step-000000.img
	for f in images/step-??????.img; do echo $$f; visualise/visualise $$f; done

dem: images images/step-000000.img
	for f in images/step-??????.img; do echo $$f; visualise/demographics $$f; done

hist: images images/step-000000.dem
	for f in images/step-??????.dem; do echo $$f; visualise/histogram.py $${f%.dem}-hist.png < $$f; done

plot: output images/apocalypse.out
	visualise/plot.py <output/apocalypse.out

.PHONY: all clean localclean localclobber clobber 
.PHONY: backup globalise globalise-images globalise-output
.PHONY: png dem hist plot
