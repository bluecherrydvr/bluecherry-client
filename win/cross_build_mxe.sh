#!/bin/bash

# Build script that cross-builds Bluecherry client
# for Windows on a GNU/Linux host using MXE build environment

# make sure that MXE dependencies are installed on a host system first
# see https://mxe.cc for details

function build_for_arch {

arch=$1

PREFIX=$PWD/usr/$arch
# MPV
cd mpv/
python ./bootstrap.py
DEST_OS=win32 TARGET=$arch ./waf configure --enable-libmpv-shared --disable-lua --disable-gl-dxinterop --disable-egl-angle-win32 --disable-egl-angle-lib --disable-cplayer --disable-manpage-build --disable-caca --disable-libavdevice --disable-libarchive --disable-libbluray  --prefix=$PREFIX || exit
./waf build
./waf install
./waf clean
cd ..

mkdir -p build_bc_client
cd build_bc_client
../../../configure --host=$arch --prefix=$PREFIX
make -j$(nproc)
make install
make clean
cd ..

#copy exe & DLLs
mkdir -p build_installer_$arch
cp $PREFIX/bin/bluecherry-client.exe build_installer_$arch/
tools/copydlldeps.sh --infile $PREFIX/bin/bluecherry-client.exe --destdir build_installer_$arch/ --recursivesrcdir $PREFIX --copy --enforcedir $PREFIX/qt5/plugins/platforms/ --enforcedir $PREFIX/qt5/plugins/imageformats/  --objdump usr/bin/$arch-objdump
#strip
usr/bin/$arch-strip build_installer_$arch/*.dll build_installer_$arch/*.exe build_installer_$arch/imageformats/*.dll build_installer_$arch/platforms/*.dll
#build installer
cd build_installer_$arch

for dll in *.dll imageformats/*.dll platforms/*.dll
do
	echo "File $dll" >> dll_filelist_$arch.nsh
done
cd ..
}


git clone https://github.com/mxe/mxe.git
git clone https://github.com/mpv-player/mpv

cd mxe
git checkout 219c3ab34978f078faf4c7ed4c091752f95a272b
patch -p1 < ../mxe.patch || exit

make MXE_TARGETS='i686-w64-mingw32.shared x86_64-w64-mingw32.shared' cc qtbase ffmpeg openssl sdl2 libass jpeg

export PATH=$PWD/usr/bin/:$PATH

# NSIS
git clone https://github.com/kichik/nsis
NSIS_PREFIX=$PWD/usr
cd nsis/
scons XGCC_W32_PREFIX=i686-w64-mingw32.shared- ZLIB_W32=$NSIS_PREFIX/i686-w64-mingw32.shared PREFIX=$NSIS_PREFIX install
cd ..

for arch in i686-w64-mingw32.shared x86_64-w64-mingw32.shared
do
	build_for_arch $arch
done
