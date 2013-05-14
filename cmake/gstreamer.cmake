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
set (USE_GSTREAMER 1)

list (APPEND bluecherry_client_LIBRARIES
    ${GSTREAMER_0_10_LIBRARIES}
)

if (NOT APPLE)
    find_package (GStreamerApp-0.10 0.10.0 REQUIRED)
    include_directories (${GSTREAMERAPP_0_10_INCLUDE_DIRS})

    list (APPEND bluecherry_client_LIBRARIES
        ${GSTREAMERAPP_0_10_LIBRARIES}
    )
endif (NOT APPLE)

if (WIN32)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set (GSTREAMER_PLUGINS ${CMAKE_SOURCE_DIR}/gstreamer-bin/win/plugins)
    else (CMAKE_BUILD_TYPE MATCHES Debug)
        set (GSTREAMER_PLUGINS plugins)
    endif (CMAKE_BUILD_TYPE MATCHES Debug)
endif (WIN32)
