#!/bin/bash

# get target triplet (machine-vendor-operatingsystem)
TARGET_TRIPLET=$(gcc -dumpmachine)

# set target arch, prefix and nsis script filename
if test $TARGET_TRIPLET = 'i686-w64-mingw32'
then
    ARCH=i686
    PREFIX=/mingw32
    LIB_SSL=libssl-1_1.dll
    NSI_FILE=installer32.nsi
elif test $TARGET_TRIPLET = 'x86_64-w64-mingw32'
then
    ARCH=x86_64
    PREFIX=/mingw64
    LIB_SSL=libssl-1_1-x64.dll
    NSI_FILE=installer64.nsi
fi

#
cd ../../

mkdir -p build-bluecherry-client-installer-msys2_$ARCH
cp build-bluecherry-client-msys2_$ARCH/win/$NSI_FILE build-bluecherry-client-installer-msys2_$ARCH/installer.nsi

cp $PREFIX/bin/bluecherry-client.exe build-bluecherry-client-installer-msys2_$ARCH/
cp $PREFIX/bin/$LIB_SSL build-bluecherry-client-installer-msys2_$ARCH/

# run copydlldeps.sh from destination dir, otherwise it recursively walks all MXE tree and copies excess DLLs
cd build-bluecherry-client-installer-msys2_$ARCH
../bluecherry-client/msys2/mxe/tools/copydlldeps.sh --infile ./bluecherry-client.exe --destdir ./ --srcdirs $PREFIX/bin --copy --enforcedir $PREFIX/share/qt5/plugins/platforms/ --enforcedir $PREFIX/share/qt5/plugins/imageformats/
cd ..

#strip
strip build-bluecherry-client-installer-msys2_$ARCH/*.dll build-bluecherry-client-installer-msys2_$ARCH/*.exe build-bluecherry-client-installer-msys2_$ARCH/imageformats/*.dll build-bluecherry-client-installer-msys2_$ARCH/platforms/*.dll


#build installer
cd build-bluecherry-client-installer-msys2_$ARCH

for dll in *.dll
do
	echo "File $dll" >> dll_list.nsh
done

# copy license file
cp ../bluecherry-client/COPYING ./

# copy icon file
cp ../bluecherry-client/res/bluecherry.ico ./

# copy translation files
cp $PREFIX/share/bluecherry-client/translations/*.qm ./

makensis installer.nsi

cd ..
