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

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set (CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -m32")

find_library (QUARTZ_CORE_LIBRARY QuartzCore)
find_library (VIDEO_DECODE_ACCELERATION_LIBRARY VideoDecodeAcceleration)
find_library (APP_KIT_LIBRARY AppKit)
find_library (CORE_SERVICES_LIBRARY CoreServices)
find_library (OPENGL_LIBRARY OpenGL)

list (APPEND bluecherry_client_LIBRARIES ${CORE_SERVICES_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${VIDEO_DECODE_ACCELERATION_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${COCOA_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${APP_KIT_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${OPENGL_LIBRARY})

set (MACOSX_BUNDLE_INFO_STRING "Bluecherry Client")
set (MACOSX_BUNDLE_ICON_FILE "bluecherry.icns")
set (MACOSX_BUNDLE_GUI_IDENTIFIER "Bluecherry Client")
set (MACOSX_BUNDLE_LONG_VERSION_STRING ${VERSION})
set (MACOSX_BUNDLE_BUNDLE_NAME "Bluecherry Client")
set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION})
set (MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
set (MACOSX_BUNDLE_COPYRIGHT "Copyright 2010-2013 Bluecherry")
