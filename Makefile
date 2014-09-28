DATE=$(shell date +"%F_%T")

all:
	$(MAKE) -C apocalypse all
	$(MAKE) -C visualise all
	$(MAKE) -C report all
	mkdir -p images

clean:
	$(MAKE) -C apocalypse clean
	$(MAKE) -C visualise clean
	$(MAKE) -C report clean

localclean:

clobber: localclean
	$(MAKE) -C apocalypse clobber
	$(MAKE) -C visualise clobber
	$(MAKE) -C report clobber
	rm -rf images/
	rm -f out
	
backup: images out
	mv images images_$(DATE)
	mv out out_$(DATE)

png: images
	for f in images/*.img; do echo $$f; visualise/visualise $$f; done

plot: out
	visualise/plot.py <out

.PHONY: all clean localclean clobber backup png plot
