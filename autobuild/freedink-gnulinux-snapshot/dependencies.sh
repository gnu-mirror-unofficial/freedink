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
# Dependencies
##
apt-get -y install wget ca-certificates \
  libfreetype6-dev libfontconfig1-dev

##
# FreeDink
##
apt-get -y install build-essential file pkg-config upx-ucl

##
# Reprotest
##
apt-get -y install disorderfs python3 python3-pip locales-all faketime \
  openssh-server

##
# AppImage
##
# build.sh
apt-get -y install git-core ca-certificates \
   libfuse-dev libglib2.0-dev cmake fuse python
