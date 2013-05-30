# - Find libavutil
# Find the BreakpadClient includes and library
# This module defines
#  LIBAVUTIL_INCLUDE_DIRS, where to find avutil.h, etc.
#  LIBAVUTIL_LIBRARIES, the libraries needed to use libavutil.
#  LIBAVUTIL_FOUND, If false, do not try to use libavutil.
# also defined, but not for general use are
#  LIBAVUTIL_LIBRARY, where to find the JPEG library.

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
        pkg_check_modules (LIBAVUTIL QUIET libavutil)
    endif (PKG_CONFIG_FOUND)
endif (NOT WIN32)

find_path (LIBAVUTIL_INCLUDE_DIR libavutil/avutil.h ${LIBAVUTIL_INCLUDE_DIRS} ${WIN32_LIBAV_DIR}/include)
list (APPEND LIBAVUTIL_INCLUDE_DIRS ${LIBAVUTIL_INCLUDE_DIR})
find_library (LIBAVUTIL_LIBRARY NAMES avutil HINTS ${LIBAVUTIL_LIBDIR} ${LIBAVUTIL_LIBRARY_DIRS} ${WIN32_LIBAV_DIR}/bin)
list (APPEND LIBAVUTIL_LIBRARIES ${LIBAVUTIL_LIBRARY})

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (LibAVUtil DEFAULT_MSG LIBAVUTIL_LIBRARIES LIBAVUTIL_INCLUDE_DIRS)
set (LIBAVUTIL_FOUND ${LibAVUtil_FOUND})

if (LIBAVUTIL_FOUND)
    set (LIBAVUTIL_LIBRARIES ${LIBAVUTIL_LIBRARY})
endif (LIBAVUTIL_FOUND)

mark_as_advanced (LIBAVUTIL_INCLUDE_DIRS LIBAVUTIL_LIBRARIES)
