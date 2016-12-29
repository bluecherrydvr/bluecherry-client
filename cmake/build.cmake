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

configure_file (src/bluecherry-config.h.in src/bluecherry-config.h)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/src)

if (UNIX AND NOT APPLE)
    set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wundef -Wcast-align -Wpointer-arith -Woverloaded-virtual -Wnon-virtual-dtor  ${CMAKE_CXX_FLAGS}")

    if (CMAKE_COMPILER_IS_GNUCC)
        execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                        OUTPUT_VARIABLE GCC_VERSION)
        if (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
            set (CMAKE_CXX_FLAGS "-Wno-unused-local-typedefs ${CMAKE_CXX_FLAGS}")
        endif ()
    endif (CMAKE_COMPILER_IS_GNUCC)

    if (DEVBUILD)
        set (CMAKE_CXX_FLAGS "-Werror -fno-omit-frame-pointer  ${CMAKE_CXX_FLAGS}")
    endif (DEVBUILD)

endif (UNIX AND NOT APPLE)

add_executable (bluecherry-client WIN32 MACOSX_BUNDLE ${bluecherry_client_SRCS} src/main.cpp)
target_link_libraries (bluecherry-client ${bluecherry_client_LIBRARIES})
set_property (TARGET bluecherry-client PROPERTY INSTALL_RPATH_USE_LINK_PATH FALSE)
set_property (TARGET bluecherry-client PROPERTY INSTALL_RPATH /usr/lib/bluecherry/client )

if (UNIX AND NOT APPLE)
    add_dependencies(bluecherry-client ffmpeg mplayer)
endif()
