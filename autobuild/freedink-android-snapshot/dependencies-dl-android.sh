#!/bin/bash -ex
# Install build dependencies in reproducible build environment

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

# Configuration
echo 'APT::Install-Recommends "false";' > /etc/apt/apt.conf.d/00InstallRecommends
# Note: don't use backports, versions may change at any time

# NDK is compiled as 32-bit
dpkg --add-architecture i386
apt-get update
apt-get -y install libc6:i386 zlib1g:i386

##
# Android
##
# https://wiki.libsdl.org/Android
apt-get -y install openjdk-7-jdk ant
apt-get -y install wget ca-certificates p7zip unzip

cd /usr/src/
wget http://android-rebuilds.beuc.net/dl/android-ndk-r10e-linux-x86.bin
wget http://android-rebuilds.beuc.net/dl/android-sdk_eng.android_linux-x86-6.0.1r31.zip
wget http://android-rebuilds.beuc.net/dl/sdk-repo-linux-tools-24.3.4.zip

7zr x android-ndk-r10e-linux-x86.bin
unzip android-sdk_eng.android_linux-x86-6.0.1r31.zip
mv android-sdk_eng.android_linux-x86/ android-sdk-linux/
unzip sdk-repo-linux-tools-24.3.4.zip -d android-sdk-linux/
chmod -R o+rX /usr/src/android-sdk-linux/

# Make room, we're exceeding the default 10G docker limit
cd /usr/src/
rm -f android-ndk-r10e-linux-x86.bin
rm -f android-sdk_eng.android_linux-x86-6.0.1r31.zip
rm -f sdk-repo-linux-tools-24.3.4.zip
rm -rf android-ndk-r10e-linux-x86.bin/sources/  # apparently safe
