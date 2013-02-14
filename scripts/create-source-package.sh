#!/bin/bash

# Copyright 2013 Bluecherry
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

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

PACKAGE_NAME=$PRODUCT-$VERSION
PACKAGE_DIR=$PACKAGE_NAME
PACKAGE_FILE=$PACKAGE_NAME.tar.bz2

git clone $GIT_URL $PACKAGE_DIR
pushd $PACKAGE_DIR
git checkout $VERSION
git submodule init
git submodule update
find -name ".git*" | xargs rm -rf
popd

tar cjf $PACKAGE_FILE $PACKAGE_DIR
md5sum $PACKAGE_FILE > $PACKAGE_FILE.md5
sha1sum $PACKAGE_FILE > $PACKAGE_FILE.sha1

echo "Package is available as $PACKAGE_NAME.tar.bz2"
