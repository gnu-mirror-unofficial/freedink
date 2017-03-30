# Reproducible build

.PHONY: all
VERSION=$(shell cd ../../native/ && make -s version)
all:
	docker build -t freedink-android:snapshot .
	if [ ! -e wd/ ]; then mkdir wd/; fi
	chown 1000:1000 wd/
	cp -a build.sh wd/
	(cd ../../native/ && make dist)
	mv ../../native/freedink-$(VERSION).tar.gz wd/
	docker run -it -v "$$(pwd)/wd:/home/repro/wd" -v "$$(pwd)/../..:/usr/src/freedink" \
	   freedink-android:snapshot /home/repro/wd/build.sh \
	     $(VERSION) $$(git log --format="%ct" -n 1)
	ls -lh wd/freedink-$(VERSION)/android/bin/freedink-debug.apk
