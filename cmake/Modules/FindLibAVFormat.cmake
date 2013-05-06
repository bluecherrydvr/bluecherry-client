# - Find libavformat
# Find the BreakpadClient includes and library
# This module defines
#  LIBAVFORMAT_INCLUDE_DIRS, where to find avformat.h, etc.
#  LIBAVFORMAT_LIBRARIES, the libraries needed to use libavformat.
#  LIBAVFORMAT_FOUND, If false, do not try to use libavformat.
# also defined, but not for general use are
#  LIBAVFORMAT_LIBRARY, where to find the JPEG library.

#
# Copyright 2010-2013 Bluecherry
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
#

if (NOT WIN32)
    find_package (PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules (LIBAVFORMAT QUIET libavformat)
    endif (PKG_CONFIG_FOUND)
endif (NOT WIN32)

find_path (LIBAVFORMAT_INCLUDE_DIR libavformat/avformat.h ${LIBAVFORMAT_INCLUDE_DIRS} ${WIN32_LIBAV_DIR}/include)
list (APPEND LIBAVFORMAT_INCLUDE_DIRS ${LIBAVFORMAT_INCLUDE_DIR})
find_library (LIBAVFORMAT_LIBRARY NAMES avformat HINTS ${LIBAVFORMAT_LIBDIR} ${LIBAVFORMAT_LIBRARY_DIRS} ${WIN32_LIBAV_DIR}/lib)
list (APPEND LIBAVFORMAT_LIBRARIES ${LIBAVFORMAT_LIBRARY})

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (LibAVFormat DEFAULT_MSG LIBAVFORMAT_LIBRARIES LIBAVFORMAT_INCLUDE_DIRS)
set (LIBAVFORMAT_0_10_FOUND ${LibAVFormat_0_10_FOUND})

if (LibAVFormat_0_10_FOUND)
    set (LIBAVFORMAT_LIBRARIES ${LIBAVFORMAT_LIBRARY})
endif (LibAVFormat_0_10_FOUND)

mark_as_advanced (LIBAVFORMAT_INCLUDE_DIRS LIBAVFORMAT_LIBRARIES)
