#!/bin/bash -ex
# MS Woe release, MXE rebuild

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

cd /opt/
git clone -n https://github.com/mxe/mxe.git
cd mxe/
git reset --hard a26337b6898e35b9c224c48da8ac6d55db83eb16

make -j2 JOBS=$(nproc) gcc
# Fixup libstdc++.a ordering
(
    cd /opt/mxe/usr/lib/gcc/i686-w64-mingw32.static/5.4.0/
    mkdir t && cd t/
    ar x ../libstdc++.a
    rm -f ../libstdc++.a
    ar Dvcr ../libstdc++.a $(ls | LC_ALL=C sort)
    cd ..
    rm -rf t/
)

make -j2 JOBS=$(nproc) sdl2 sdl2_gfx sdl2_image sdl2_mixer sdl2_ttf glm libzip gettext
