#!/bin/bash -ex
# MS Woe release, built with MXE

# Copyright (C) 2008, 2009, 2010, 2012, 2013, 2014, 2017  Sylvain Beucler

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

if [ $# -ne 2 ]; then
    echo "Usage: $0 package version epoch"
    exit 1
fi

PACKAGE=freedink
VERSION=$1
SOURCE_DATE_EPOCH=$2

export SOURCE_DATE_EPOCH
umask 022
export LANG=C.UTF8
unset LC_ALL LANGUAGE

##
# Build from tarball
##

rm -rf $PACKAGE-$VERSION/
tar xzf $PACKAGE-$VERSION.tar.gz
pushd $PACKAGE-$VERSION/
mkdir zip/

# documentation
for i in TRANSLATIONS.txt; do
    cp $i zip/$PACKAGE-$i
done
for i in AUTHORS COPYING NEWS README THANKS TROUBLESHOOTING; do
    cp $i zip/$PACKAGE-$i.txt
done
cat <<EOF > zip/$PACKAGE-SOURCE.txt
The FreeDink source code is available at:
  http://ftp.gnu.org/gnu/freedink/

The source code is the "recipe" of FreeDink, that let you study it,
modify it, and redistribute your changes.  The GNU GPL license
explicitely allows you to do so (see $PACKAGE-COPYING.txt).

If you upload a FreeDink .exe on your website, you must also offer the
corresponding source code for download.
EOF

# Include documentation with MS-DOS newlines (if not already)
sed -i -e 's/\(^\|[^\r]\)$/\1\r/' zip/$PACKAGE-*.txt


# full-static
PATH=/opt/mxe/usr/bin:$PATH

rm -rf cross-w32/
mkdir cross-w32/
pushd cross-w32/
# Reproducible build:
# --no-insert-timestamp + strip is *almost* reproducible with this version of binutils
# http://blog.beuc.net/posts/Practical_basics_of_reproducible_builds_2/
../configure --host=i686-w64-mingw32.static \
  --enable-static --enable-upx --disable-tests \
  LDFLAGS='-Wl,--no-insert-timestamp'
make -j $(nproc)
make install-strip DESTDIR=$(pwd)/destdir
# move .exe but avoid symlinks
find destdir/usr/local/bin/ -type f -name "*.exe" | while read file; do
  mv $file ../zip/
done
# Resources
cp -a destdir/usr/local/share/freedink ../zip/
cp -a destdir/usr/local/share/locale ../zip/freedink/
popd  # cross-w32/

# Set reproducible date for all generated files:
#find zip/ -newermt "@${SOURCE_DATE_EPOCH}" -print0 \
#  | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
# Timestamps are from Git here (snapshot), so replace them for all files:
find zip/ -print0 \
  | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"

rm -f ../$PACKAGE-$VERSION-bin.zip
# Reproducible build:
# TZ=UTC: avoid MS-DOS timestamp variations due to timezone
#   https://wiki.debian.org/ReproducibleBuilds/TimestampsInZip
# -X: strip platform-specific info (timestamps, uid/gid, permissions)
# sort file list https://wiki.debian.org/ReproducibleBuilds/FileOrderInTarballs
(cd zip/ && find . -print0 \
  | LC_ALL=C sort -z \
  | TZ=UTC xargs -0 -n10 \
    zip -X ../../$PACKAGE-$VERSION-bin.zip)
popd  # $PACKAGE-$VERSION/

# Alternatively we might use strip-nondeterminism:
# Sort file list and insert timezone-independent timestamp
#strip-nondeterminism -T $SOURCE_DATE_EPOCH $PACKAGE-$VERSION-bin.zip
# ^ stuck in the '80s until Stretch is stable, no -T
#strip-nondeterminism $PACKAGE-$VERSION-bin.zip
