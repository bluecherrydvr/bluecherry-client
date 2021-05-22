Bluecherry cross platform video surveillance client application

https://www.bluecherrydvr.com

Copyright 2010-2021 Bluecherry, LLC

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.


Dependencies.
=========================
Required dependencies for Bluecherry client are:
- Qt 5
- SDL 2.0.5 or newer
- FFmpeg 4.0 or newer
- MPV (libmpv library)



Building from sources.
=========================

git clone https://github.com/bluecherrydvr/bluecherry-client.git
cd bluecherry-client
./autogen.sh
./configure
make
make install

Building Debian packages.
=========================
Install dependency packages and developer tools first:

sudo apt-get install autoconf automake build-essential debhelper devscripts
g++ git libasound2-dev libavdevice-dev libavfilter-dev libmpv-dev libsdl2-dev
libva-dev qtbase5-dev qttools5-dev-tools qt5-default

then build the package:

git clone https://github.com/bluecherrydvr/bluecherry-client.git
cd bluecherry-client
debuild -uc -us -sn -b

Building for Windows.
=========================

Preferable way of building is cross-compiling on a GNU/Linux host
using MinGW compiler. MXE project provides convenient way
of building MinGW cross-toolchain and many opensource libraries
required as dependencies.

1. Make sure MXE requirements are installed on your host system.
See https://mxe.cc/#requirements

2. Download Bluecherry sources and build dependencies, Bluecherry
client .exe and Windows installer using provided helper script:

git clone https://github.com/bluecherrydvr/bluecherry-client.git
cd bluecherry-client
./autogen.sh
cd win
./cross_build_mxe.sh

Once build is complete (may take few hours), installer executables
should be located in
win/mxe/build_installer_i686-w64-mingw32.shared/
and
win/mxe/build_installer_x86_64-w64-mingw32.shared/
for 32 and 64 bit Windows respectively.

Building for OS X.
=========================

Dependencies can be provided by Macports, Homebrew
or you can build them yourself.

After the configure and make steps, run

make deploy

to create .dmg package file.

