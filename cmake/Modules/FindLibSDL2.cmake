# - Find libSDK2
# Find the SDL2 includes and library
# This module defines
#  LIBSDL2_I:%NCLUDE_DIRS, where to find swscale.h, etc.
#  LIBSDL2_LIBRARIES, the libraries needed to use libswscale.
#  LIBSDL2_FOUND, If false, do not try to use libswscale.
# also defined, but not for general use are
#  LIBSDL2_LIBRARY, where to find the JPEG library.

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

if (NOT WIN32 AND NOT LIBSDL2_INCLUDE_DIRS)
    find_package (PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules (LIBSDL2 QUIET sdl2)
    endif (PKG_CONFIG_FOUND)
endif ()

find_path (LIBSDL2_INCLUDE_DIR SDL2/SDL.h ${LIBSDL2_INCLUDE_DIRS} ${WIN32_LIBSDL2_DIR}/include)
list (APPEND LIBSDL2_INCLUDE_DIRS ${LIBSDL2_INCLUDE_DIR})
find_library (LIBSDL2_LIBRARY NAMES SDL2 HINTS ${LIBSDL2_LIBDIR} ${LIBSDL2_LIBRARY_DIRS} ${WIN32_LIBSDL2_DIR}/bin)
list (APPEND LIBSDL2_LIBRARIES ${LIBSDL2_LIBRARY})

if (UNIX AND NOT APPLE)
	list (APPEND LIBSDL2_LIBRARIES "-ldl" "-lpthread" "-lasound" "-lm")
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (LibSDL2 DEFAULT_MSG LIBSDL2_LIBRARIES LIBSDL2_INCLUDE_DIRS)
set (LIBSDL2_FOUND ${LibSDL2_FOUND})

if (LIBSDL2_FOUND)
    set (LIBSDL2_LIBRARIES ${LIBSDL2_LIBRARY})
endif (LIBSDL2_FOUND)


mark_as_advanced (LIBSDL2_INCLUDE_DIRS LIBSDL2_LIBRARIES)

