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


find_library (QUARTZ_CORE_LIBRARY QuartzCore)
find_library (VIDEO_DECODE_ACCELERATION_LIBRARY VideoDecodeAcceleration)
find_library (APP_KIT_LIBRARY AppKit)
find_library (CORE_SERVICES_LIBRARY CoreServices)
find_library (OPENGL_LIBRARY OpenGL)
find_library (COREAUDIO_LIBRARY CoreAudio)
find_library (AUDIOTOOLBOX_LIBRARY AudioToolbox)
find_library (AUDIOUNIT_LIBRARY AudioUnit)

list (APPEND bluecherry_client_LIBRARIES ${CORE_SERVICES_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${VIDEO_DECODE_ACCELERATION_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${APP_KIT_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${OPENGL_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${COREAUDIO_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${AUDIOTOOLBOX_LIBRARY})
list (APPEND bluecherry_client_LIBRARIES ${AUDIOUNIT_LIBRARY})

set (MACOSX_BUNDLE_INFO_STRING "Bluecherry Client")
set (MACOSX_BUNDLE_ICON_FILE "bluecherry.icns")
set (MACOSX_BUNDLE_GUI_IDENTIFIER "Bluecherry Client")
set (MACOSX_BUNDLE_LONG_VERSION_STRING ${VERSION})
set (MACOSX_BUNDLE_BUNDLE_NAME "Bluecherry Client")
set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION})
set (MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
set (MACOSX_BUNDLE_COPYRIGHT "Copyright 2010-2019 Bluecherry, LLC")

add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bluecherry-client.app/Contents/Resources/qt.conf
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_SOURCE_DIR}/mac/deploy.sh ${CMAKE_CURRENT_BINARY_DIR}/bluecherry-client.app ${QT_BINARY_DIR}/macdeployqt
)

add_custom_target (deploy
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bluecherry-client.app/Contents/Resources/qt.conf
)
