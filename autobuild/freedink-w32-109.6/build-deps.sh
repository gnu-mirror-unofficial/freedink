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
git reset --hard 023da395bf995e67830a6456e1b576788375ead0

# Reproducible build
# MXE has patched binutils with linker SOURCE_DATE_EPOCH support
# This should trigger some reproducible support
export SOURCE_DATE_EPOCH=$(cd /opt/mxe && git log --format="%ct" -n 1)

make -j2 JOBS=$(nproc) gcc
# Reproducible build
# Fixup libstdc++.a ordering; would require stable ordering in the GCC build system
(
    cd /opt/mxe/usr/lib/gcc/i686-w64-mingw32.static/5.5.0/
    mkdir t && cd t/
    ar x ../libstdc++.a
    rm -f ../libstdc++.a
    ar Dvcr ../libstdc++.a $(ls | LC_ALL=C sort)
    cd ..
    rm -rf t/
)

make -j2 JOBS=$(nproc) sdl2 sdl2_gfx sdl2_image sdl2_mixer sdl2_ttf glm libzip gettext
