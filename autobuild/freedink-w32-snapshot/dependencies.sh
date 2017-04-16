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

apt-get update

export DEBIAN_FRONTEND=noninteractive

##
# MXE
##
# http://mxe.cc/#requirements
apt-get -y install git-core ca-certificates
apt-get -y install \
    autoconf automake autopoint bash bison bzip2 flex gettext\
    git g++ gperf intltool libffi-dev libgdk-pixbuf2.0-dev \
    libtool libltdl-dev libssl-dev libxml-parser-perl make \
    openssl p7zip-full patch perl pkg-config python ruby scons \
    sed unzip wget xz-utils
apt-get -y install g++-multilib libc6-dev-i386
apt-get -y install libtool-bin

##
# FreeDink
##
apt-get -y install upx
apt-get -y install zip

##
# Reprotest
##
apt-get -y install disorderfs python3 python3-pip locales-all faketime
apt-get -y install openssh-server
