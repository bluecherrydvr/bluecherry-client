# - Find GStreamerApp-0.10
# Find the GStreamerApp includes and library
# This module defines
#  GSTREAMERAPP_0_10_INCLUDE_DIRS, where to find gst.h, etc.
#  GSTREAMERAPP_0_10_LIBRARIES, the libraries needed to use GStreamer.
#  GSTREAMERAPP_0_10_FOUND, If false, do not try to use GStreamer.
# also defined, but not for general use are
#  GSTREAMERAPP_0_10_LIBRARY, where to find the JPEG library.

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

if (NOT WIN32)
    find_package (PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules (GSTREAMERAPP_0_10 QUIET gstreamer-app-0.10)
    endif (PKG_CONFIG_FOUND)
endif (NOT WIN32)

find_path (GSTREAMERAPP_0_10_INCLUDE_DIR gst/app/gstappsrc.h PATHS ${WIN32_GSTREAMERAPP_DIR}/include PATH_SUFFIXES gstreamer-0.10/)
list (APPEND GSTREAMERAPP_0_10_INCLUDE_DIRS ${GSTREAMERAPP_0_10_INCLUDE_DIR})

if (UNIX)
    find_library (GSTREAMERAPP_0_10_LIBRARY NAMES gstapp-0.10 HINTS ${GSTREAMERAPP_0_10_LIBDIR} ${GSTREAMERAPP_0_10_LIBRARY_DIRS} ${WIN32_GSTREAMERAPP_DIR}/lib)
    set (GSTREAMERAPP_0_10_LIBRARIES
        ${GSTREAMERAPP_0_10_LIBRARY}
    )
elseif (WIN32)
endif (UNIX)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GStreamer-App-0.10 DEFAULT_MSG GSTREAMERAPP_0_10_LIBRARY GSTREAMERAPP_0_10_LIBRARIES GSTREAMERAPP_0_10_INCLUDE_DIRS)

mark_as_advanced (GSTREAMERAPP_0_10_INCLUDE_DIRS GSTREAMERAPP_0_10_LIBRARIES)
