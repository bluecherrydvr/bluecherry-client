#!/bin/bash

# print current MSYSTEM
echo "==== MSYSTEM: $MSYSTEM ===="

# set SRC_DIR, BUILD_DIR and BUILD_INSTALLER_DIR variables
SRC_DIR=bluecherry-client
BUILD_DIR=build-$SRC_DIR-MSYS2-$MSYSTEM
BUILD_INSTALLER_DIR=build-$SRC_DIR-installer-MSYS2-$MSYSTEM

# create build directory outside from source directory
cd ../../

mkdir -p $BUILD_INSTALLER_DIR

# copy files
if [ "MINGW32" = $MSYSTEM ] 
then
    cp $BUILD_DIR/win/installer32.nsi $BUILD_INSTALLER_DIR/installer.nsi
    cp $MSYSTEM_PREFIX/bin/libssl-1_1.dll $BUILD_INSTALLER_DIR/
elif [ "MINGW64" = $MSYSTEM ]
then
    cp $BUILD_DIR/win/installer64.nsi $BUILD_INSTALLER_DIR/installer.nsi
    cp $MSYSTEM_PREFIX/bin/libssl-1_1-x64.dll $BUILD_INSTALLER_DIR/
else
    echo "ERROR: Unknown MSYSTEM: $MSYSTEM"
    exit 1
fi

cp $MSYSTEM_PREFIX/bin/bluecherry-client.exe $BUILD_INSTALLER_DIR/
cp $MSYSTEM_PREFIX/share/bluecherry-client/translations/*.qm $BUILD_INSTALLER_DIR/
cp $SRC_DIR/COPYING $BUILD_INSTALLER_DIR/
cp $SRC_DIR/res/bluecherry.ico $BUILD_INSTALLER_DIR/

# copy dlls, strip, build installer 
cd $BUILD_INSTALLER_DIR

../$SRC_DIR/msys2/mxe/tools/copydlldeps.sh --infile ./bluecherry-client.exe --destdir ./ --srcdirs $MSYSTEM_PREFIX/bin --copy --enforcedir $MSYSTEM_PREFIX/share/qt5/plugins/platforms/ --enforcedir $MSYSTEM_PREFIX/share/qt5/plugins/imageformats/
strip *.dll *.exe imageformats/*.dll platforms/*.dll

rm -f dll_list.nsh

for dll in *.dll
do
	echo "File $dll" >> dll_list.nsh
done

makensis installer.nsi
