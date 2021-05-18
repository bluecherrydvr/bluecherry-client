#!/bin/bash

# Build script that cross-builds Bluecherry client
# for Windows on a GNU/Linux host using MXE build environment

# make sure that MXE dependencies are installed on a host system first
# see https://mxe.cc for details

git clone https://github.com/mxe/mxe.git

cd mxe
git checkout 219c3ab34978f078faf4c7ed4c091752f95a272b
patch -p1 < ../mxe.patch || exit

make MXE_TARGETS='i686-w64-mingw32.shared' cc qtbase ffmpeg openssl sdl2 libass jpeg

export PATH=$PWD/usr/bin/:$PATH

PREFIX=$PWD/usr/i686-w64-mingw32.shared
# MPV
git clone https://github.com/mpv-player/mpv
cd mpv/
python ./bootstrap.py
DEST_OS=win32 TARGET=i686-w64-mingw32.shared ./waf configure --enable-libmpv-shared --disable-lua --disable-gl-dxinterop --disable-egl-angle-win32 --disable-egl-angle-lib --disable-cplayer --disable-manpage-build --disable-caca --disable-libavdevice --disable-libarchive --disable-libbluray  --prefix=$PREFIX || exit
./waf build
./waf install
cd ..

# NSIS
git clone https://github.com/kichik/nsis
cd nsis/
scons XGCC_W32_PREFIX=i686-w64-mingw32.shared- ZLIB_W32=$PREFIX PREFIX=$PREFIX install
cd ..

mkdir build_bc_client
cd build_bc_client
../../../configure --host=i686-w64-mingw32.shared --prefix=$PREFIX CXXFLAGS=-std=c++11
make -j$(nproc)
make install

#copy exe & DLLs
#strip
#build installer

