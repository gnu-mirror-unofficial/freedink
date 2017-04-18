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

# disable MP3 support (patents expire at the end of 2017)
sed -i -e 's/--enable-music-mp3/--disable-music-mp3/' src/sdl2_mixer.mk
# Reproducible build
# Stable 0 timestamps - https://github.com/mxe/mxe/pull/1737
cat <<EOF > src/binutils-1-reproducible_timestamp.patch
http://sourceware.org/git/gitweb.cgi?p=binutils-gdb.git;a=commit;f=bfd/peXXigen.c;h=1c5f704fc035bc705dee887418f42cb8bca24b5d

Ensure that the timestamp in PE/COFF headers is always initialised.

PR ld/20634
* peXXigen.c (_bfd_XXi_only_swap_filehdr_out): Put 0 in the
timestamp field if real time values are not being stored.

diff -ru binutils-2.25.1/bfd/peXXigen.c binutils-2.25.1b/bfd/peXXigen.c
--- binutils-2.25.1/bfd/peXXigen.c	2015-07-21 10:20:58.000000000 +0200
+++ binutils-2.25.1b/bfd/peXXigen.c	2017-03-31 11:41:39.140582071 +0200
@@ -875,6 +875,8 @@
   /* Only use a real timestamp if the option was chosen.  */
   if ((pe_data (abfd)->insert_timestamp))
     H_PUT_32 (abfd, time (0), filehdr_out->f_timdat);
+  else
+    H_PUT_32 (abfd, 0, filehdr_out->f_timdat);
 
   PUT_FILEHDR_SYMPTR (abfd, filehdr_in->f_symptr,
 		      filehdr_out->f_symptr);
EOF

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
