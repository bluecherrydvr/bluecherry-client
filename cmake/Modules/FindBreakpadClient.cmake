# - Find BreakpadClient
# Find the BreakpadClient includes and library
# This module defines
#  BREAKPADCLIENT_INCLUDE_DIR, where to find exception_handler.h, etc.
#  BREAKPADCLIENT_LIBRARIES, the libraries needed to use BreakpadClient.
#  BREAKPADCLIENT_FOUND, If false, do not try to use BreakpadClient.
# also defined, but not for general use are
#  BREAKPADCLIENT_LIBRARY, where to find the JPEG library.

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

find_path (BREAKPADCLIENT_INCLUDE_DIR_${CMAKE_BUILD_TYPE} client/linux/handler/exception_handler.h ${WIN32_BREAKPAD_SRC_DIR} ${MACOSX_BREAKPAD_SRC_DIR} ${LINUX_BREAKPAD_DIR}/src)

if (UNIX AND NOT APPLE)
    find_library (BREAKPADCLIENT_LIBRARY_${CMAKE_BUILD_TYPE} NAMES libbreakpad_client.a PATHS ${LINUX_BREAKPAD_DIR}/src/client/linux/)
    set (BREAKPADCLIENT_LIBRARIES_${CMAKE_BUILD_TYPE}
        ${BREAKPADCLIENT_LIBRARY_${CMAKE_BUILD_TYPE}}
    )
elseif (WIN32)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set (WIN32_BREAKPAD_LIB_SUFFIX debug)
    else (CMAKE_BUILD_TYPE MATCHES Debug)
        set (WIN32_BREAKPAD_LIB_SUFFIX release)
    endif (CMAKE_BUILD_TYPE MATCHES Debug)
    set (WIN32_BREAKPAD_LIB_DIR ${WIN32_BREAKPAD_DIR}/lib-${WIN32_BREAKPAD_LIB_SUFFIX})

    find_library (BREAKPADCLIENT_COMMON_LIBRARY_${CMAKE_BUILD_TYPE} NAMES common.lib PATHS ${WIN32_BREAKPAD_LIB_DIR})
    find_library (BREAKPADCLIENT_CRASH_GENERATION_CLIENT_LIBRARY_${CMAKE_BUILD_TYPE} NAMES crash_generation_client.lib PATHS ${WIN32_BREAKPAD_LIB_DIR})
    find_library (BREAKPADCLIENT_EXCEPTION_HANDLER_LIBRARY_${CMAKE_BUILD_TYPE} NAMES exception_handler.lib PATHS ${WIN32_BREAKPAD_LIB_DIR})
    set (BREAKPADCLIENT_LIBRARIES_${CMAKE_BUILD_TYPE}
        ${BREAKPADCLIENT_COMMON_LIBRARY_${CMAKE_BUILD_TYPE}}
        ${BREAKPADCLIENT_CRASH_GENERATION_CLIENT_LIBRARY_${CMAKE_BUILD_TYPE}}
        ${BREAKPADCLIENT_EXCEPTION_HANDLER_LIBRARY_${CMAKE_BUILD_TYPE}}
    )
elseif (APPLE)
    find_library (BREAKPADCLIENT_FRAMEWORK NAMES Breakpad PATHS ${MACOSX_BREAKPAD_BIN_DIR})

    set (BREAKPADCLIENT_LIBRARIES_${CMAKE_BUILD_TYPE}
        ${BREAKPADCLIENT_FRAMEWORK}
    )
endif (UNIX AND NOT APPLE)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (BreakpadClient DEFAULT_MSG BREAKPADCLIENT_LIBRARIES_${CMAKE_BUILD_TYPE} BREAKPADCLIENT_INCLUDE_DIR_${CMAKE_BUILD_TYPE})

mark_as_advanced (BREAKPADCLIENT_LIBRARY_${CMAKE_BUILD_TYPE} BREAKPADCLIENT_LIBRARIES_${CMAKE_BUILD_TYPE} BREAKPADCLIENT_INCLUDE_DIR_${CMAKE_BUILD_TYPE})

