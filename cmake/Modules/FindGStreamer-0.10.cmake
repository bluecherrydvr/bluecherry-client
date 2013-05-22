# - Find GStreamer-0.10
# Find the GStreamer includes and library
# This module defines
#  GSTREAMER_0_10_INCLUDE_DIRS, where to find gst.h, etc.
#  GSTREAMER_0_10_LIBRARIES, the libraries needed to use GStreamer.
#  GSTREAMER_0_10_FOUND, If false, do not try to use GStreamer.
# also defined, but not for general use are
#  GSTREAMER_0_10_LIBRARY, where to find the JPEG library.

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
        pkg_check_modules (GSTREAMER_0_10 gstreamer-0.10)
    endif (PKG_CONFIG_FOUND)
endif (NOT WIN32)

if (UNIX AND NOT APPLE)
    find_library (GSTREAMER_0_10_LIBRARY NAMES gstreamer-0.10 HINTS ${GSTREAMER_0_10_LIBDIR} ${GSTREAMER_0_10_LIBRARY_DIRS} ${WIN32_GSTREAMER_DIR}/lib)
elseif (WIN32)
    find_path (GSTREAMER_0_10_INCLUDE_DIR gst/gst.h PATHS ${WIN32_GSTREAMER_DIR}/include PATH_SUFFIXES gstreamer-0.10/)

    find_library (GSTREAMER_0_10_LIBRARY NAMES gstreamer-0.10.lib PATHS ${WIN32_GSTREAMER_DIR}/lib)
    find_library (GSTINTERFACES_0_10_LIBRARY NAMES gstinterfaces-0.10.lib PATHS ${WIN32_GSTREAMER_DIR}/lib)
    find_library (GSTAPP_0_10_LIBRARY NAMES gstapp-0.10.lib PATHS ${WIN32_GSTREAMER_DIR}/lib)
    find_library (GSTVIDEO_0_10_LIBRARY NAMES gstvideo-0.10.lib PATHS ${WIN32_GSTREAMER_DIR}/lib)
    find_library (GLIB_2_0_LIBRARY NAMES glib-2.0.lib PATHS ${WIN32_GSTREAMER_DIR}/lib)
    find_library (GOBJECT_2_0_LIBRARY NAMES gobject-2.0.lib PATHS ${WIN32_GSTREAMER_DIR}/lib)

    set (GSTREAMER_0_10_LIBRARIES
        ${GSTREAMER_0_10_LIBRARY}
        ${GSTINTERFACES_0_10_LIBRARY}
        ${GSTAPP_0_10_LIBRARY}
        ${GSTVIDEO_0_10_LIBRARY}
        ${GLIB_2_0_LIBRARY}
        ${GOBJECT_2_0_LIBRARY}
    )
elseif (APPLE)
    find_path (GSTREAMER_0_10_INCLUDE_DIR gst/gst.h PATHS ${MACOSX_GSTREAMER_DIR}/include PATH_SUFFIXES gstreamer-0.10/)

    find_library (GSTREAMER_0_10_LIBRARY NAMES gstreamer-0.10.0 PATHS ${MACOSX_GSTREAMER_DIR}/lib)
    find_library (GSTINTERFACES_0_10_LIBRARY NAMES gstinterfaces-0.10.0 PATHS ${MACOSX_GSTREAMER_DIR}/lib)
    find_library (GSTAPP_0_10_LIBRARY NAMES gstapp-0.10.0 PATHS ${MACOSX_GSTREAMER_DIR}/lib)
    find_library (GSTVIDEO_0_10_LIBRARY NAMES gstvideo-0.10.0 PATHS ${MACOSX_GSTREAMER_DIR}/lib)

    set (GSTREAMER_0_10_LIBRARIES
        ${GSTREAMER_0_10_LIBRARY}
        ${GSTINTERFACES_0_10_LIBRARY}
        ${GSTAPP_0_10_LIBRARY}
        ${GSTVIDEO_0_10_LIBRARY}
        ${MACOSX_GSTREAMER_DIR}/lib/libglib-2.0.0.dylib
        ${MACOSX_GSTREAMER_DIR}/lib/libgobject-2.0.0.dylib
    )
endif (UNIX AND NOT APPLE)

list (APPEND GSTREAMER_0_10_INCLUDE_DIRS ${GSTREAMER_0_10_INCLUDE_DIR})

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GStreamer-0.10 DEFAULT_MSG GSTREAMER_0_10_LIBRARY GSTREAMER_0_10_LIBRARIES GSTREAMER_0_10_INCLUDE_DIRS)

mark_as_advanced (GSTREAMER_0_10_INCLUDE_DIRS GSTREAMER_0_10_LIBRARIES)
