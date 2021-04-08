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
- MPV



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
libva-dev qtbase5-dev qttools5-dev-tools

then build the package:

git clone https://github.com/bluecherrydvr/bluecherry-client.git
cd bluecherry-client
debuild -uc -us -sn -b

