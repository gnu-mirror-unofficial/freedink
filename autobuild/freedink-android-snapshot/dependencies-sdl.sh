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
# pkgconf
##
apt-get -y install make gcc libc6-dev


PATH="/usr/src/android-ndk-r10e:$PATH"                  # for 'ndk-build'
PATH="/usr/src/android-sdk-linux/tools:$PATH"           # for 'android'
PATH="/usr/src/android-sdk-linux/platform-tools:$PATH"  # for 'adb'

##
# Compile a shared binaries bundle for SDL and SDL_*
##

# Start with a minimal build: 
cd /usr/src/SDL2/
cd build-scripts/
#hg revert --all  # remove traces of previous builds
sed -i androidbuild.sh -e 's/$ANDROID update project --path $BUILDPATH/& --target android-23/'
./androidbuild.sh org.libsdl /dev/null || true
# doesn't matter if the actual build fails, it's just for setup
cd ../build/org.libsdl/

# Remove reference to our dummy file
rm -rf jni/src/

# Reference SDL_image, SDL_mixer, SDL_ttf, and their dependencies, as NDK modules:
ln -s /usr/src/SDL2_image jni/
ln -s /usr/src/SDL2_image/external/libwebp-0.3.0 jni/webp
ln -s /usr/src/SDL2_mixer jni/
ln -s /usr/src/SDL2_mixer/external/libmikmod-3.1.12 jni/libmikmod
ln -s /usr/src/SDL2_mixer/external/smpeg2-2.0.0 jni/smpeg2
ln -s /usr/src/SDL2_net jni/
ln -s /usr/src/SDL2_ttf jni/

# Optionnaly edit `jni/Android.mk` to disable some formats, e.g.:
sed -i -e '1ecat' jni/Android.mk <<EOF
SUPPORT_MP3_SMPEG := false
EOF

# Launch the build!
ndk-build -j$(nproc)

##
# Install SDL in a GCC toolchain
##

# Copy the NDK into a traditional GCC toolchain (leave android-14 as-is):
/usr/src/android-ndk-r10e/build/tools/make-standalone-toolchain.sh \
  --platform=android-14 --install-dir=/usr/src/ndk-standalone-14-arm --arch=arm

# Set your PATH (important, do it before any build):
NDK_STANDALONE=/usr/src/ndk-standalone-14-arm
PATH=$NDK_STANDALONE/bin:$PATH

# Install the SDL2 binaries in the toolchain:
cd /usr/src/SDL2/build/org.libsdl/
for i in libs/armeabi/*; do ln -nfs $(pwd)/$i $NDK_STANDALONE/sysroot/usr/lib/; done
mkdir $NDK_STANDALONE/sysroot/usr/include/SDL2/
\cp jni/SDL/include/* $NDK_STANDALONE/sysroot/usr/include/SDL2/
\cp jni/*/SDL*.h $NDK_STANDALONE/sysroot/usr/include/SDL2/

# Install `pkg-config` and install a host-triplet-prefixed symlink in the PATH (auto-detected by autoconf):
VERSION=0.9.12
cd /usr/src/
wget http://rabbit.dereferenced.org/~nenolod/distfiles/pkgconf-$VERSION.tar.gz
tar xf pkgconf-$VERSION.tar.gz
cd pkgconf-$VERSION/
mkdir native-android/ && cd native-android/
../configure --prefix=$NDK_STANDALONE/sysroot/usr
make -j$(nproc)
make install
ln -s ../sysroot/usr/bin/pkgconf $NDK_STANDALONE/bin/arm-linux-androideabi-pkg-config
mkdir $NDK_STANDALONE/sysroot/usr/lib/pkgconfig/

# Install pkg-config `.pc` files for SDL:
cat <<'EOF' > $NDK_STANDALONE/sysroot/usr/lib/pkgconfig/sdl2.pc
prefix=/usr/src/ndk-standalone-14-arm/sysroot/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include
Name: sdl2
Description: Simple DirectMedia Layer is a cross-platform multimedia library designed to provide low level access to audio, keyboard, mouse, joystick, 3D hardware via OpenGL, and 2D video framebuffer.
Version: 2.0.5
Requires:
Conflicts:
Libs: -lSDL2
Cflags: -I${includedir}/SDL2   -D_REENTRANT
EOF

cat <<'EOF' > $NDK_STANDALONE/sysroot/usr/lib/pkgconfig/SDL2_image.pc
prefix=/usr/src/ndk-standalone-14-arm/sysroot/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include
Name: SDL2_image
Description: image loading library for Simple DirectMedia Layer
Version: 2.0.1
Requires: sdl2 >= 2.0.0
Libs: -L${libdir} -lSDL2_image
Cflags: -I${includedir}/SDL2
EOF

cat <<'EOF' > $NDK_STANDALONE/sysroot/usr/lib/pkgconfig/SDL2_mixer.pc
prefix=/usr/src/ndk-standalone-14-arm/sysroot/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include
Name: SDL2_mixer
Description: mixer library for Simple DirectMedia Layer
Version: 2.0.1
Requires: sdl2 >= 2.0.0
Libs: -L${libdir} -lSDL2_mixer
Cflags: -I${includedir}/SDL2
EOF

cat <<'EOF' > $NDK_STANDALONE/sysroot/usr/lib/pkgconfig/SDL2_net.pc
prefix=/usr/src/ndk-standalone-14-arm/sysroot/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include
Name: SDL2_net
Description: net library for Simple DirectMedia Layer
Version: 2.0.1
Requires: sdl2 >= 2.0.0
Libs: -L${libdir} -lSDL2_net
Cflags: -I${includedir}/SDL2
EOF

cat <<'EOF' > $NDK_STANDALONE/sysroot/usr/lib/pkgconfig/SDL2_ttf.pc
prefix=/usr/src/ndk-standalone-14-arm/sysroot/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include
Name: SDL2_ttf
Description: ttf library for Simple DirectMedia Layer with FreeType 2 support
Version: 2.0.14
Requires: sdl2 >= 2.0.0
Libs: -L${libdir} -lSDL2_ttf
Cflags: -I${includedir}/SDL2
EOF


##
# Building other dependencies
##

cd /usr/src/

# SDL2_gfx
VERSION=1.0.3
wget http://www.ferzkopp.net/Software/SDL2_gfx/SDL2_gfx-$VERSION.tar.gz
tar xf SDL2_gfx-$VERSION.tar.gz
mv SDL2_gfx-$VERSION/ SDL2_gfx/
pushd SDL2_gfx/
mkdir cross-android/ && cd cross-android/
../configure --host=arm-linux-androideabi --prefix=$NDK_STANDALONE/sysroot/usr \
  --disable-shared --disable-mmx
make -j$(nproc)
make install
popd

# gettext
VERSION=0.19.5
wget -c http://ftp.gnu.org/pub/gnu/gettext/gettext-$VERSION.tar.gz
tar xf gettext-$VERSION.tar.gz
pushd gettext-$VERSION/
cd gettext-runtime/
mkdir cross-android/ && cd cross-android/
../configure --host=arm-linux-androideabi --prefix=$NDK_STANDALONE/sysroot/usr \
  --disable-shared
make -j$(nproc) && make install
popd

# glm
VERSION=0.9.8.4
wget -c https://github.com/g-truc/glm/archive/$VERSION.tar.gz -O glm-$VERSION.tar.gz
tar xf glm-$VERSION.tar.gz
pushd glm-$VERSION/
cp -a glm $NDK_STANDALONE/sysroot/usr/include/
popd

# cxxtest - not used for docker build but useful for developer setup
VERSION=4.4
wget https://sourceforge.net/projects/cxxtest/files/cxxtest/4.4/cxxtest-$VERSION.tar.gz/download -O cxxtest-$VERSION.tar.gz
tar xf cxxtest-$VERSION.tar.gz
pushd cxxtest-$VERSION/
cp -a cxxtest $NDK_STANDALONE/sysroot/usr/include/
popd


##
# Packaged FreeDink build dependencies
##
apt-get -y install upx
apt-get -y install zip strip-nondeterminism file
