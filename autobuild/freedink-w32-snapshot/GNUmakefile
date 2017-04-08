# Reproducible build

.PHONY: all
VERSION=$(shell cd ../../native/ && make -s version)
all:
	docker build -t freedink-w32:snapshot .
	if [ ! -e wd/ ]; then mkdir wd/; fi
	chown 1000:1000 wd/
	chmod 00755 wd/
	cp -a build.sh wd/
	(cd ../../native/ && make dist)
	mv ../../native/freedink-$(VERSION).tar.gz wd/
	docker run -it -v "$$(pwd)/wd:/home/repro/wd" \
	   freedink-w32:snapshot /home/repro/wd/build.sh \
	     $(VERSION) $$(git log --format="%ct" -n 1)
	ls -lh wd/freedink-$(VERSION)-bin.zip
	sha256sum wd/freedink-$(VERSION)-bin.zip
