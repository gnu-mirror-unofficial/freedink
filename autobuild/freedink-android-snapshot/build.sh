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

PATH="/usr/src/android-ndk-r10e:$PATH"                  # for 'ndk-build'
PATH="/usr/src/android-sdk-linux/tools:$PATH"           # for 'android'
PATH="/usr/src/android-sdk-linux/platform-tools:$PATH"  # for 'adb'

PATH=/usr/src/ndk-standalone-14-arm/bin:$PATH
mkdir cross-android/
pushd cross-android/
../configure --host=arm-linux-androideabi \
  --prefix=/android-aint-posix \
  --disable-tests
make -j$(nproc)
make install DESTDIR=$(pwd)/destdir  # for locales
popd

pushd android/
mkdir -p libs/armeabi/
cp -a /usr/src/SDL2/build/org.libsdl/libs/armeabi/* libs/armeabi/
make SDK_TARGET=23
popd
