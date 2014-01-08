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

find_package (GStreamer-0.10 0.10.0 REQUIRED)
include_directories (${GSTREAMER_0_10_INCLUDE_DIRS})

list (APPEND bluecherry_client_LIBRARIES
    ${GSTREAMER_0_10_LIBRARIES}
)

if (UNIX AND NOT APPLE)
    find_package (GStreamerApp-0.10 0.10.0 REQUIRED)
    include_directories (${GSTREAMERAPP_0_10_INCLUDE_DIRS})

    list (APPEND bluecherry_client_LIBRARIES
        ${GSTREAMERAPP_0_10_LIBRARIES}
    )

    set (GSTREAMER_PLUGIN_PATHS "${GSTREAMER_PLUGIN_PATHS}")
    set (GSTREAMER_PLUGIN_PREFIX "lib")
    set (GSTREAMER_PLUGIN_SUFFIX ".so")
    set (GSTREAMER_PLUGINS "gsttypefindfunctions:gstapp:gstdecodebin2:gstmatroska:gstffmpegcolorspace:gstcoreelements:gstffmpeg")

endif (UNIX AND NOT APPLE)

if (APPLE)
    set (GSTREAMER_PLUGIN_PATHS "./../PlugIns/gstreamer/:${GSTREAMER_PLUGIN_PATHS}")
    set (GSTREAMER_PLUGIN_PREFIX "lib")
    set (GSTREAMER_PLUGIN_SUFFIX ".so")
    set (GSTREAMER_PLUGINS "gsttypefindfunctions:gstapp:gstdecodebin2:gstmatroska:gstffmpegcolorspace:gstcoreelements:gstffmpeg:gstosxaudio")
endif (APPLE)

if (WIN32)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set (GSTREAMER_PLUGIN_PATHS "${CMAKE_SOURCE_DIR}/gstreamer-bin/win/plugins:${GSTREAMER_PLUGIN_PATHS}")
    else (CMAKE_BUILD_TYPE MATCHES Debug)
        set (GSTREAMER_PLUGIN_PATHS "./plugins:${GSTREAMER_PLUGIN_PATHS}")
    endif (CMAKE_BUILD_TYPE MATCHES Debug)

    set (GSTREAMER_PLUGIN_PREFIX "lib")
    set (GSTREAMER_PLUGIN_SUFFIX ".dll")
    set (GSTREAMER_PLUGINS "gsttypefindfunctions:gstapp:gstdecodebin2:gstmatroska:gstffmpegcolorspace:gstcoreelements:gstffmpeg-lgpl:gstautodetect:gstaudioconvert:gstaudioresample:gstvolume:gstdirectsoundsink")
endif (WIN32)
