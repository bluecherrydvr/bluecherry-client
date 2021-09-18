#!/bin/bash

# print current MSYSTEM
echo "==== MSYSTEM: $MSYSTEM ===="

# set SRC_DIR and BUILD_DIR variables
SRC_DIR=bluecherry-client
BUILD_DIR=build-$SRC_DIR-MSYS2-$MSYSTEM

# run autoreconf
cd ../

./autogen.sh

# create build directory outside from source directory
cd ../

mkdir -p $BUILD_DIR

# configure, build and install 
cd $BUILD_DIR

../$SRC_DIR/configure
make -j $(nproc)
make install
