# - Find libswscale
# Find the BreakpadClient includes and library
# This module defines
#  LIBSWSCALE_INCLUDE_DIRS, where to find swscale.h, etc.
#  LIBSWSCALE_LIBRARIES, the libraries needed to use libswscale.
#  LIBSWSCALE_FOUND, If false, do not try to use libswscale.
# also defined, but not for general use are
#  LIBSWSCALE_LIBRARY, where to find the JPEG library.

#
# Copyright 2010-2019 Bluecherry, LLC
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

if (NOT WIN32 AND NOT LIBSWSCALE_INCLUDE_DIRS)
    find_package (PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules (LIBSWSCALE QUIET libswscale)
    endif (PKG_CONFIG_FOUND)
endif ()

find_path (LIBSWSCALE_INCLUDE_DIR libswscale/swscale.h ${LIBSWSCALE_INCLUDE_DIRS} ${WIN32_LIBAV_DIR}/include)
list (APPEND LIBSWSCALE_INCLUDE_DIRS ${LIBSWSCALE_INCLUDE_DIR})
find_library (LIBSWSCALE_LIBRARY NAMES swscale HINTS ${LIBSWSCALE_LIBDIR} ${LIBSWSCALE_LIBRARY_DIRS} ${WIN32_LIBAV_DIR}/bin)
list (APPEND LIBSWSCALE_LIBRARIES ${LIBSWSCALE_LIBRARY})

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (LibSWScale DEFAULT_MSG LIBSWSCALE_LIBRARIES LIBSWSCALE_INCLUDE_DIRS)
set (LIBSWSCALE_FOUND ${LibSWScale_FOUND})

if (LIBSWSCALE_FOUND)
    set (LIBSWSCALE_LIBRARIES ${LIBSWSCALE_LIBRARY})
endif (LIBSWSCALE_FOUND)

mark_as_advanced (LIBSWSCALE_INCLUDE_DIRS LIBSWSCALE_LIBRARIES)
