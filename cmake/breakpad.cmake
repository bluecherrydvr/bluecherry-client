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

find_package (BreakpadClient REQUIRED)

list (APPEND bluecherry_client_SRCS
    src/utils/Breakpad.cpp
)

include_directories (${BREAKPADCLIENT_INCLUDE_DIR_${CMAKE_BUILD_TYPE}})
set (USE_BREAKPAD 1)

if (UNIX)
    set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -gstabs)
    list (APPEND CMAKE_INCLUDE_PATH ${LINUX_BREAKPAD_DIR})
elseif (WIN32)
    set (CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} /DEBUG)
    list (APPEND CMAKE_INCLUDE_PATH ${WIN32_BREAKPAD_SRC_DIR})
endif (UNIX)

list (APPEND bluecherry_client_LIBRARIES
    ${BREAKPADCLIENT_LIBRARIES_${CMAKE_BUILD_TYPE}}
)
