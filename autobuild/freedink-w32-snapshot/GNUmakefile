# Reproducible build MS Woe release environment and tests

# Copyright (C) 2017  Sylvain Beucler

# This file is part of GNU FreeDink

# GNU FreeDink is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# GNU FreeDink is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see
# <http://www.gnu.org/licenses/>.

.PHONY: all
PACKAGE=freedink
VERSION=$(shell cd ../../native/ && make -s version)
SOURCE_DATE_EPOCH=$(shell git log --format="%ct" -n 1)

all: wd/$(PACKAGE)-$(VERSION).tar.gz
	docker build -t $(PACKAGE)-w32:snapshot .
	docker run -it -v "$$(pwd)/wd:/home/repro/wd" \
	   $(PACKAGE)-w32:snapshot /home/repro/wd/build.sh \
	     $(VERSION) $(SOURCE_DATE_EPOCH)


# Test with reprotest - sadly the build path changes and cannot be configured
# => different BuildID than direct build
reprotest-docker: clean-build wd/$(PACKAGE)-$(VERSION).tar.gz ssh/authorized_keys
	# Run in docker through SSH
	# --security-opt seccomp=unconfined: kernel (linux32/linux64)
	# --cap-add SYS_ADMIN --device /dev/fuse: fileordering
	cid=$$(docker run -d -ti -u root \
	    --cap-add SYS_ADMIN --device /dev/fuse \
	    --security-opt seccomp=unconfined \
	    -p 127.0.0.1:10022:22 -v "$$(pwd)/ssh:/home/repro/.ssh" $(PACKAGE)-w32:snapshot \
            sh -c 'mkdir /run/sshd && /usr/sbin/sshd -D') \
	  && reprotest --store-dir wd/reprotest/ \
	    'cd wd/ && ../build.sh $(VERSION) $(SOURCE_DATE_EPOCH)' \
	    'wd/$(PACKAGE)-$(VERSION)-bin.zip' \
	    -- ssh -p 10022 -H repro@localhost -i ssh/id_rsa**** \
	  ; echo -n "Killing Docker container: "; docker kill $$cid

reprotest-chroot: clean-build wd/$(PACKAGE)-$(VERSION).tar.gz
	#[ -d ../chroot/ ] || sudo debootstrap stretch ../chroot/
	#sudo cp -a dependencies.sh build-deps.sh ../chroot/usr/src/
	#sudo chroot ../chroot /usr/src/dependencies.sh
	#sudo chroot ../chroot /usr/src/build-deps.sh
	# fileordering:
	#sudo mknod -m 666 ../chroot/dev/fuse c 10 229
	# sem_open: Function not implemented:
	-sudo chroot ../chroot umount /dev/shm
	sudo chroot ../chroot mount tmpfs -t tmpfs /dev/shm
	reprotest --store-dir wd/reprotest/ \
	  'cd wd/ && ../build.sh $(VERSION) $(SOURCE_DATE_EPOCH)' \
	  'wd/$(PACKAGE)-$(VERSION)-bin.zip' \
	  -- chroot -r sudo $$(pwd)/../chroot/
	sudo chroot ../chroot umount /dev/shm

reprotest-null: clean-build wd/$(PACKAGE)-$(VERSION).tar.gz
	reprotest --store-dir wd/reprotest/ \
	  'cd wd/ && ../build.sh $(VERSION) $(SOURCE_DATE_EPOCH)' \
	  'wd/$(PACKAGE)-$(VERSION)-bin.zip'


ssh/authorized_keys:
	mkdir -p ssh/
	ssh-keygen -f ssh/id_rsa -N ''
	cp -a ssh/id_rsa.pub ssh/authorized_keys

wd/$(PACKAGE)-$(VERSION).tar.gz:
	if [ ! -e wd/ ]; then mkdir wd/; fi
	chown 1000:1000 wd/
	chmod 00755 wd/
	cp -a build.sh wd/
	(cd ../../native/ && make dist)
	mv ../../native/$(PACKAGE)-$(VERSION).tar.gz wd/

clean-build:
	# reprotest doesn't like the symlinks from `make install`
	rm -rf wd/$(PACKAGE)-$(VERSION)/
	# and wants an empty store-dir
	rm -rf wd/reprotest/
clean-tarball:
	rm -f wd/$(PACKAGE)-$(VERSION).tar.gz
clean:
	rm -rf wd/
