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

if (UNIX AND NOT APPLE)
	include (GNUInstallDirs)

	configure_file ("linux/bluecherry-client.desktop.in" "linux/bluecherry-client.desktop")

	install (TARGETS bluecherry-client RUNTIME DESTINATION bin)
	install (FILES "${CMAKE_CURRENT_BINARY_DIR}/linux/bluecherry-client.desktop" DESTINATION share/applications)
	install (FILES "res/bluecherry-client.png" DESTINATION share/icons)

        install (FILES ${TRANSLATION_FILES} DESTINATION "${CMAKE_INSTALL_DATADIR}/bluecherry-client/translations")

	# mplayer
	install( PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/install/usr/bin/mplayer"
		DESTINATION bin
		RENAME bc-mplayer
	)
	# ffmpeg
	install( DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/install/usr/lib/bluecherry"
		DESTINATION lib
		USE_SOURCE_PERMISSIONS
	)

endif (UNIX AND NOT APPLE)
