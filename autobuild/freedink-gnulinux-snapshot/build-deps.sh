#!/bin/bash -ex
# Install build dependencies in reproducible build environment

# Copyright (C) 2015, 2017  Sylvain Beucler

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


##
# Building other dependencies
##

cd /usr/src/
(
    cd SDL2/
    ./configure
    make -j$(nproc)
    make install
)

# SDL2_image
# - no PNG support for now
# - no JPG support for now
(
    cd SDL2_image/
    ./configure
    make -j$(nproc)
    make install
)

# SDL2_mixer
# - assume libogg is installed on the target system
# - assume libfluidsynth is installed on the target system for now
#   (depends on glib/gthread + requires large sound fonts)
# - provide libmodplug
# - no MP3 support (patents issues until 2018)
# - no Flac support (don't enable formats unless needed and documented)
(
    cd libmodplug/
    ./configure
    make -j$(nproc)
    make install
)
(
    cd SDL2_mixer/
    ./configure \
        --enable-music-ogg-shared \
        --enable-music-midi-fluidsynth-shared \
        --enable-music-mod-modplug \
        --disable-music-flac \
        --disable-music-mp3
    make -j$(nproc)
    make install
)

# SDL2_ttf
# - assume freetype6 installed on the target system for now
(
    cd SDL2_ttf/
    ./configure
    # Ditch example that depends on -lGL
    echo 'void main(){}' > glfont.c
    make -j$(nproc) glfont_LDADD=''
    make install
)

(
    cd SDL2_gfx/
    ./configure
    make -j$(nproc)
    make install
)

(
    cd glm/
    cp -a glm /usr/include/
)

# AppImageKit
(
    cd AppImageKit/
    ./build.sh  # beware, control the packages it installs...
)
