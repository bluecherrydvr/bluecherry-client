#!/bin/bash

# get target triplet (machine-vendor-operatingsystem)
TARGET_TRIPLET=$(gcc -dumpmachine)

# set target arch
if test $TARGET_TRIPLET = 'i686-w64-mingw32'
then
    ARCH=i686
elif test $TARGET_TRIPLET = 'x86_64-w64-mingw32'
then
    ARCH=x86_64
fi

# run autoreconf
cd ../

./autogen.sh

# create build directory outside from source directory
cd ../

mkdir -p build-bluecherry-client-msys2_$ARCH

# configure, build and install 
cd build-bluecherry-client-msys2_$ARCH

../bluecherry-client/configure
make -j $(nproc)
make install
