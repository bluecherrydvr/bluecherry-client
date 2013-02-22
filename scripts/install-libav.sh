#!/bin/bash

LIBAV_SRC_VERSION=9.1
LIBAV_SRC_FILE=libav-$LIBAV_SRC_VERSION.tar.gz
LIBAV_SRC_DIR=libav-$LIBAV_SRC_VERSION
LIBAV_SRC_URL=http://libav.org/releases/$LIBAV_SRC_FILE

function installLibAv {
    installBuildDependencies
    downloadLibAvSources
    unpackLibAvSources

    pushd $LIBAV_SRC_DIR
    buildLibAvFromSources
    installLibAvFromSources
    popd
}

function installBuildDependencies {
    sudo apt-get install yasm
}

function downloadLibAvSources {
    wget $LIBAV_SRC_URL
}

function unpackLibAvSources {
    tar xzf $LIBAV_SRC_FILE
}

function buildLibAvFromSources {
    ./configure --prefix=/usr/lib/bluecherry/client/ --enable-shared
    make -j5
}

function installLibAvFromSources {
    sudo make install
}

installLibAv
