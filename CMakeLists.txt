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

project (bluecherry-client)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

cmake_minimum_required (VERSION 2.8.0)
cmake_policy (SET CMP0000 OLD)

if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 8 AND ${CMAKE_PATCH_VERSION} GREATER 3) 
    cmake_policy (SET CMP0017 NEW)
endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 8 AND ${CMAKE_PATCH_VERSION} GREATER 3) 

option (DEVBUILD "Turn on some compiler flags helpful during development process" OFF)
option (ENABLE_BREAKPAD "Enable Breakpad integration" ON)
option (BUILD_TESTING "Build the test suite" ON)

if (NOT DEFINED CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
        if (DEVBUILD)
                set (default Debug)
        else (DEVBUILD)
                set (default RelWithDebInfo)
        endif (DEVBUILD)

        set (CMAKE_BUILD_TYPE "${default}")

endif (NOT DEFINED CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)


# hack for < 2.8.0 CMake
if ("${CMAKE_CURRENT_LIST_DIR}" STREQUAL "")
    set (CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
endif ("${CMAKE_CURRENT_LIST_DIR}" STREQUAL "")
set (CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules
    ${CMAKE_ROOT}/Modules
)

if (EXISTS ${CMAKE_SOURCE_DIR}/user.cmake)
    include (${CMAKE_SOURCE_DIR}/user.cmake)
endif (EXISTS ${CMAKE_SOURCE_DIR}/user.cmake)

set (bluecherry_client_SRCS "")
set (bluecherry_client_LIBRARIES "")

include (cmake/version.cmake)
include (cmake/sources.cmake)

include (cmake/qt.cmake)
include (cmake/moc_sources.cmake)
include (cmake/resources.cmake)

include (cmake/libav.cmake)
#include (cmake/gstreamer.cmake)

if (ENABLE_BREAKPAD)
    include (cmake/breakpad.cmake)
endif (ENABLE_BREAKPAD)

if (BUILD_TESTING)
    include (cmake/tests/test.cmake)
endif (BUILD_TESTING)

if (MSVC)
    include (cmake/msvc.cmake)
endif (MSVC)

if (WIN32)
    include (cmake/win32.cmake)
endif (WIN32)

if (APPLE)
    include (cmake/macosx.cmake)
endif (APPLE)

include (cmake/build.cmake)
include (cmake/translations.cmake)
include (cmake/install.cmake)
include (cmake/display-status.cmake)
