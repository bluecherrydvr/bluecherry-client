#!/bin/bash

PRODUCT=bluecherry-client
VERSION=$1
GIT_URL=git://github.com/vogel/bluecherry-client.git

function usage
{
        echo "Usage: $0 package-version"
        echo "    package-version must be a git tag name"
}

if [ "!" == "!$VERSION" ]; then
        usage
        exit
fi

sudo apt-get install build-essential devscripts debhelper
sudo apt-get install git-core
sudo apt-get install libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev

DEBIAN_PACKAGE_NAME=${PRODUCT}_${VERSION}
DEBIAN_PACKAGE_DIR=${PRODUCT}-${VERSION}
DEBIAN_PACKAGE_FILE=$DEBIAN_PACKAGE_NAME.orig.tar.gz

git clone $GIT_URL $DEBIAN_PACKAGE_DIR
pushd $DEBIAN_PACKAGE_DIR
git checkout $VERSION
git submodule init
git submodule update
find -name ".git*" | xargs rm -rf

debuild --prepend-path /usr/lib/bluecherry/qt4.8/bin -us -uc -sn -b -j5
popd
