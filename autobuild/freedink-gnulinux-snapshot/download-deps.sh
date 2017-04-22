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

##
# SDL
##

cd /usr/src/
wget https://libsdl.org/release/SDL2-2.0.5.tar.gz
wget https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.1.tar.gz
wget https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.1.tar.gz
wget https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.14.tar.gz
wget http://www.ferzkopp.net/Software/SDL2_gfx/SDL2_gfx-1.0.3.tar.gz

tar xf SDL2-2.0.5.tar.gz
tar xf SDL2_image-2.0.1.tar.gz
tar xf SDL2_mixer-2.0.1.tar.gz
tar xf SDL2_ttf-2.0.14.tar.gz
tar xf SDL2_gfx-1.0.3.tar.gz

ln -s SDL2-2.0.5 SDL2
ln -s SDL2_image-2.0.1 SDL2_image
ln -s SDL2_mixer-2.0.1 SDL2_mixer
ln -s SDL2_ttf-2.0.14 SDL2_ttf
ln -s SDL2_gfx-1.0.3 SDL2_gfx

wget https://downloads.sourceforge.net/project/modplug-xmms/libmodplug/0.8.8.5/libmodplug-0.8.8.5.tar.gz
tar xf libmodplug-0.8.8.5.tar.gz
ln -s libmodplug-0.8.8.5 libmodplug

wget -c https://github.com/g-truc/glm/archive/0.9.8.4.tar.gz -O glm-0.9.8.4.tar.gz
tar xf glm-0.9.8.4.tar.gz
ln -s glm-0.9.8.4 glm

git clone --depth 1 https://github.com/probonopd/AppImageKit -b stable/v1.0
