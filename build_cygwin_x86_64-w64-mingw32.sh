#!/bin/bash

export PATH="/usr/lib/qt4/x86_64-w64-mingw32/bin:$PATH"

cp x86_64-w64-mingw32_user.cmake user.cmake

rm build_w64 -rf
mkdir build_w64
cd build_w64

cp ../x86_64-w64-mingw32.cmake ./

cmake  -DCMAKE_TOOLCHAIN_FILE=x86_64-w64-mingw32.cmake  -DCMAKE_BUILD_TYPE=Release -DWIN32=true ..

make


