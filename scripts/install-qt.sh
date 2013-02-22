#!/bin/bash

QT_SRC_VERSION=4.8.4
QT_SRC_FILE=qt-everywhere-opensource-src-$QT_SRC_VERSION.tar.gz
QT_SRC_DIR=qt-everywhere-opensource-src-$QT_SRC_VERSION
QT_SRC_URL=http://releases.qt-project.org/qt4/source/$QT_SRC_FILE

function installQt {
    installBuildDependencies
    downloadQtSources
    unpackQtSources

    pushd $QT_SRC_DIR
    buildQtFromSources
    installQtFromSources
    popd
}

function installBuildDependencies {
    # for openssl support
    sudo apt-get install libssl-dev
    # for opengl support
    sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
    # for webkit support
    sudo apt-get install libxrender-dev
    # for gtkstyle
    sudo apt-get install libgtk2.0-dev
}

function downloadQtSources {
    wget $QT_SRC_URL
}

function unpackQtSources {
    tar xzf $QT_SRC_FILE
}

function buildQtFromSources {
    ./configure -prefix /usr/lib/bluecherry/qt4.8/ -opensource -confirm-license -no-qt3support -openssl -opengl desktop -webkit -gtkstyle -qtlibinfix -bluecherry
    make -j5
}

function installQtFromSources {
    sudo make install
}

installQt
