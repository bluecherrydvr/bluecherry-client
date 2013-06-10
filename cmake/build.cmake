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
    set (CMAKE_CXX_FLAGS "-Werror -Wall -Wextra -Wundef -Wcast-align -Wpointer-arith -Woverloaded-virtual -Wnon-virtual-dtor ${CMAKE_CXX_FLAGS}")
endif (UNIX AND NOT APPLE)

add_executable (bluecherry-client WIN32 MACOSX_BUNDLE ${bluecherry_client_SRCS} src/main.cpp)
target_link_libraries (bluecherry-client ${bluecherry_client_LIBRARIES})
set_property (TARGET bluecherry-client PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
