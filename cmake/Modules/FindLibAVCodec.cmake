# - Find libavcodec
# Find the BreakpadClient includes and library
# This module defines
#  LIBAVCODEC_INCLUDE_DIRS, where to find avcodec.h, etc.
#  LIBAVCODEC_LIBRARIES, the libraries needed to use libavcodec.
#  LIBAVCODEC_FOUND, If false, do not try to use libavcodec.
# also defined, but not for general use are
#  LIBAVCODEC_LIBRARY, where to find the JPEG library.

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
        pkg_check_modules (LIBAVCODEC QUIET libavcodec)
    endif (PKG_CONFIG_FOUND)
endif (NOT WIN32)

find_path (LIBAVCODEC_INCLUDE_DIR libavcodec/avcodec.h PATHS ${LIBAVCODEC_INCLUDE_DIRS} ${WIN32_LIBAV_DIR}/include)
list (APPEND LIBAVCODEC_INCLUDE_DIRS ${LIBAVCODEC_INCLUDE_DIR})
find_library (LIBAVCODEC_LIBRARY NAMES avcodec HINTS ${LIBAVCODEC_LIBDIR} ${LIBAVCODEC_LIBRARY_DIRS} ${WIN32_LIBAV_DIR}/lib)
list (APPEND LIBAVCODEC_LIBRARIES ${LIBAVCODEC_LIBRARY})

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (LibAVCodec DEFAULT_MSG LIBAVCODEC_LIBRARIES LIBAVCODEC_INCLUDE_DIRS)
set (LIBAVCODEC_0_10_FOUND ${LibAVCodec_0_10_FOUND})

if (LibAVCodec_0_10_FOUND)
    set (LIBAVCODEC_LIBRARIES ${LIBAVCODEC_LIBRARY})
endif (LibAVCodec_0_10_FOUND)

mark_as_advanced (LIBAVCODEC_INCLUDE_DIRS LIBAVCODEC_LIBRARIES)
