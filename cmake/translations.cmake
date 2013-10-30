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

message (STATUS " * trans direcgorry: ${CMAKE_CURRENT_SOURCE_DIR}/translations")

file (GLOB_RECURSE TRANSLATION_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ts")

qt4_add_translation (TRANSLATION_FILES ${TRANSLATION_SOURCES})

add_custom_target (translations DEPENDS ${TRANSLATION_FILES})
add_dependencies (bluecherry-client translations)

