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

set(TRANSLATION_SOURCES translations/bluecherryclient_en.ts
	translations/bluecherryclient_de_DE.ts
	translations/bluecherryclient_es.ts
	translations/bluecherryclient_no.ts
	translations/bluecherryclient_pl.ts
	translations/bluecherryclient_ru.ts
	translations/bluecherryclient_sv_SE.ts
)

qt4_add_translation (TRANSLATION_FILES ${TRANSLATION_SOURCES})

add_custom_target (translations DEPENDS ${TRANSLATION_FILES})
add_dependencies (bluecherry-client translations)

