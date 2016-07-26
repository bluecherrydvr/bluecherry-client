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
#if ( APPLE )
#    include(cmake/CpuCoresCount.cmake)
#    include(ExternalProject)
#    include(cmake/AddExternalQt.cmake)
#endif()

set (QT_USE_QTCORE 1)
set (QT_USE_QTDECLARATIVE 1)
set (QT_USE_QTGUI 1)
set (QT_USE_QTNETWORK 1)
set (QT_USE_QTOPENGL 1)
set (QT_USE_QTWEBKIT 1)

if (UNIX AND NOT APPLE)
    set (QT_USE_QTDBUS 1)
endif (UNIX AND NOT APPLE)

if (WIN32)
    set (QT_USE_QTMAIN 1)
endif (WIN32)

if (BUILD_TESTING)
    set (QT_USE_QTTEST 1)
endif (BUILD_TESTING)

find_package (Qt4 4.8.0 REQUIRED)
include (cmake/Modules/UseQt4.cmake)

if (CMAKE_BUILD_TYPE STREQUAL Release)
    add_definitions (-DQT_NO_WARNING_OUTPUT)
    add_definitions (-DQT_NO_DEBUG_OUTPUT)
endif (CMAKE_BUILD_TYPE STREQUAL Release)

if (UNIX AND NOT APPLE)
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${QT_LIBRARY_DIR}")
endif (UNIX AND NOT APPLE)

if (APPLE)
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -F${QT_LIBRARY_DIR}")
endif (APPLE)

# add_definitions (-DQT_NO_CAST_FROM_ASCII)
add_definitions (-DQT_NO_CAST_TO_ASCII)

list (APPEND bluecherry_client_LIBRARIES
    ${QT_LIBRARIES}
)

if (WIN32)
    list (APPEND bluecherry_client_LIBRARIES
        opengl32.lib
        glu32.lib
    )
elseif (UNIX AND NOT APPLE)
    find_package (OpenGL REQUIRED)
    list (APPEND bluecherry_client_LIBRARIES
        ${OPENGL_LIBRARIES}
    )
endif (WIN32)

if (UNIX AND NOT APPLE)
    find_package (OpenGL REQUIRED)
endif (UNIX AND NOT APPLE)

list (APPEND bluecherry_client_LIBRARIES
    ${OPENGL_LIBRARIES}
)
