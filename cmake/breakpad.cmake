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

find_package (BreakpadClient REQUIRED)

list (APPEND bluecherry_client_SRCS
    src/utils/Breakpad.cpp
)

include_directories (${BREAKPADCLIENT_INCLUDE_DIR_${CMAKE_BUILD_TYPE}})
set (USE_BREAKPAD 1)

if (APPLE)
    add_custom_target (create-symbols
        COMMAND ${MACOSX_BREAKPAD_BIN_DIR}/gather_symbols.sh ${CMAKE_BINARY_DIR}/bluecherry-client
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
elseif (UNIX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gstabs")
    list (APPEND CMAKE_INCLUDE_PATH ${LINUX_BREAKPAD_DIR})

    set (BREAKPAD_DUMPSYMS ${LINUX_BREAKPAD_DIR}/src/tools/linux/dump_syms/dump_syms)

    add_custom_target (create-symbols
        COMMAND python
            ${CMAKE_SOURCE_DIR}/scripts/symbolstore.py
            ${BREAKPAD_DUMPSYMS}
            ${CMAKE_BINARY_DIR}/bluecherry-client.symbols
            ${CMAKE_BINARY_DIR}/bluecherry-client
        DEPENDS bluecherry-client
    )

elseif (WIN32)
    set (CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} /DEBUG)
    list (APPEND CMAKE_INCLUDE_PATH ${WIN32_BREAKPAD_SRC_DIR})
endif (APPLE)

list (APPEND bluecherry_client_LIBRARIES
    ${BREAKPADCLIENT_LIBRARIES_${CMAKE_BUILD_TYPE}}
)

#this has to ba added after appending breakpadclient libs
if (UNIX)
    FIND_PACKAGE ( Threads REQUIRED )
    list (APPEND bluecherry_client_LIBRARIES
        ${CMAKE_THREAD_LIBS_INIT}
    )
endif (UNIX)
