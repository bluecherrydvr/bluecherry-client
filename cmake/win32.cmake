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

configure_file (win/installer.nsi.in win/installer.nsi @ONLY)

if (MINGW)
	include(cmake/CorrectWindowsPaths.cmake)
	set(WIN32_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
	set(WIN32_CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR})
	convert_cygwin_path(WIN32_CMAKE_SOURCE_DIR)
	convert_cygwin_path(WIN32_CMAKE_BINARY_DIR)
	configure_file (win/installer64_mingw.nsi.in win/installer64_mingw.nsi @ONLY)
endif()
include_directories (${WIN32_MSINTTYPES_DIR})

list (APPEND bluecherry_client_SRCS
    res/windows.rc
)
