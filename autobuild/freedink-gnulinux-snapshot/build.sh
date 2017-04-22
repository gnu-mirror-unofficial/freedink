#!/bin/bash -ex
# GNU/Linux binary release, FreeDink itself

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

rm -rf native/
mkdir native/
pushd native/
# Reproducible build:
# - nothing yet (possibly -fdebug-prefix-map 8))
# - no UPX compression, output segfaults with 3.91-2
# Security:
# From $(dpkg-buildflags --get CXXFLAGS):
# -DFORTIFY_SOURCE=2: additional checks on string operations
# -fstack-protector-strong: use canary in more functions
# -Wformat -Werror=format-security: stop on insecure format string
../configure \
  --disable-tests \
  CXXFLAGS='-DFORTIFY_SOURCE=2 -fstack-protector-strong -Wformat -Werror=format-security'
make V=1 -j$(nproc)
make install-strip DESTDIR=$(pwd)/destdir
popd  # native/

# Set reproducible date for all generated files:
#find zip/ -newermt "@${SOURCE_DATE_EPOCH}" -print0 \
#  | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
# Timestamps are from Git here (snapshot), so replace them for all files:
find destdir/ -print0 \
  | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"

popd  # $PACKAGE-$VERSION/

# TODO: generate .appimage
