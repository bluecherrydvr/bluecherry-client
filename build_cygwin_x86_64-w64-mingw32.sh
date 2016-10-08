#!/bin/bash

export PATH="/usr/lib/qt4/x86_64-w64-mingw32/bin:$PATH"

mkdir build_w64

cp ../x86_64-w64-mingw32.cmake ./

cmake  -DCMAKE_TOOLCHAIN_FILE=x86_64-w64-mingw32.cmake  -DCMAKE_BUILD_TYPE=Release -DWIN32=true ..


