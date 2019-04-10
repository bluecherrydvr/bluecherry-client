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


if ( WIN32 OR BC_NO_BUNDLED_FFMPEG )
find_package (LibAVCodec 53.35.0 REQUIRED)
find_package (LibAVFormat 53.21.1 REQUIRED)
find_package (LibAVUtil 51.22.1 REQUIRED)
find_package (LibSWScale 2.1.0 REQUIRED)
endif()

if ( UNIX )
    include(cmake/CpuCoresCount.cmake)
    include(ExternalProject)

    if (NOT BC_NO_BUNDLED_FFMPEG)
	include(cmake/AddExternalFFmpeg.cmake)
    endif()


endif()

include_directories (${LIBAVCODEC_INCLUDE_DIRS})
include_directories (${LIBAVFORMAT_INCLUDE_DIRS})
include_directories (${LIBAVUTIL_INCLUDE_DIRS})
include_directories (${LIBSWSCALE_INCLUDE_DIRS})

if ( WIN32 )
link_directories (${LIBAVCODEC_LIBRARY_DIRS})
link_directories (${LIBAVFORMAT_LIBRARY_DIRS})
link_directories (${LIBAVUTIL_LIBRARY_DIRS})
link_directories (${LIBSWSCALE_LIBRARY_DIRS})
endif()

# __STDC_CONSTANT_MACROS is necessary for libav on Linux
add_definitions (-D__STDC_CONSTANT_MACROS)

list (APPEND bluecherry_client_LIBRARIES
    ${LIBAVCODEC_LIBRARIES}
    ${LIBAVFORMAT_LIBRARIES}
    ${LIBAVUTIL_LIBRARIES}
    ${LIBSWSCALE_LIBRARIES}
)

#get_filename_component (LIBAVCODEC_RPATH ${LIBAVCODEC_LIBRARY} PATH)
#get_filename_component (LIBAVFORMAT_RPATH ${LIBAVFORMAT_LIBRARY} PATH)
#get_filename_component (LIBAVUTIL_RPATH ${LIBAVUTIL_LIBRARY} PATH)
#get_filename_component (LIBSWSCALE_RPATH ${LIBSWSCALE_LIBRARY} PATH)

