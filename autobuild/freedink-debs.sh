#!/bin/bash -ex
# Debian snapshot

# Copyright (C) 2008, 2009, 2010, 2011, 2014  Sylvain Beucler

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

PACKAGE=freedink
VERSION=$1
if [ -z "$VERSION" ]; then
    VERSION=$(cd /mnt/snapshots/$PACKAGE && ls -d */ | sed 's,/$,,' | sort -n | tail -1)
fi

PUBDIR=/mnt/snapshots/$PACKAGE/$VERSION

rm -rf t/
mkdir t
pushd t
TARBALL=$PACKAGE-$VERSION.tar.gz
cp -a $PUBDIR/$TARBALL .
tar xzf $TARBALL
ln -s $TARBALL ${PACKAGE}_$VERSION.orig.tar.gz 
cd $PACKAGE-$VERSION/
yes | DEBEMAIL="beuc@debian.org" DEBFULLNAME="Sylvain Beucler" dch -D stable \
  --newversion $VERSION-1 \
  --force-bad-version -- \
  "New upstream release"
pdebuild --debbuildopts '-sa' --buildresult /mnt/snapshots/debian \
  -- --basetgz /var/cache/pbuilder/base-wheezy-bpo.tar.gz
popd
make -C /mnt/snapshots/debian
rm -rf t

exit;

# construction:

# with pbuilder / wheezy:
pbuilder --create --basetgz /var/cache/pbuilder/base-wheezy-bpo.tar.gz --distribution wheezy \
  --othermirror "deb http://http.debian.net/debian wheezy-backports main"
# update:
pbuilder --update --basetgz /var/cache/pbuilder/base-wheezy-bpo.tar.gz
