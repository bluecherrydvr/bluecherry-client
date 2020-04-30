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
#if ( APPLE )
#    include(cmake/CpuCoresCount.cmake)
#    include(ExternalProject)
#    include(cmake/AddExternalQt.cmake)
#endif()

find_package(Qt5 COMPONENTS Core Gui Network Widgets Quick Qml LinguistTools REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Quick_INCLUDE_DIRS})

get_target_property(QtLib_location Qt5::Core LOCATION)
list (APPEND bluecherry_client_LIBRARIES ${QtLib_location})
get_target_property(QtLib_location Qt5::Gui LOCATION)
list (APPEND bluecherry_client_LIBRARIES ${QtLib_location})
get_target_property(QtLib_location Qt5::Network LOCATION)
list (APPEND bluecherry_client_LIBRARIES ${QtLib_location})
get_target_property(QtLib_location Qt5::Widgets LOCATION)
list (APPEND bluecherry_client_LIBRARIES ${QtLib_location})
get_target_property(QtLib_location Qt5::Quick LOCATION)
list (APPEND bluecherry_client_LIBRARIES ${QtLib_location})
get_target_property(QtLib_location Qt5::Qml LOCATION)
list (APPEND bluecherry_client_LIBRARIES ${QtLib_location})

#the following line is currently needed until the code will get rid of using deprecated functions
add_definitions (-DQT_DISABLE_DEPRECATED_BEFORE=0)

if(UNIX AND NOT APPLE AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    # FIX: Qt was built with -reduce-relocations
    add_definitions(-fPIC)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Release)
    add_definitions (-DQT_NO_WARNING_OUTPUT)
    add_definitions (-DQT_NO_DEBUG_OUTPUT)
endif (CMAKE_BUILD_TYPE STREQUAL Release)

#if (UNIX AND NOT APPLE)
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${QT_LIBRARY_DIR}")
#endif (UNIX AND NOT APPLE)

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
