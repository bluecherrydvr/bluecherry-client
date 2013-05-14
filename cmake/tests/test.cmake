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

if (NOT APPLE)
    enable_testing ()
    include (cmake/tests/bluecherry-add-test.cmake)

    bluecherry_add_test (VersionTestCase tests/src/core/VersionTestCase.cpp)
    bluecherry_add_test (DateTimeRangeTestCase tests/src/utils/DateTimeRangeTestCase.cpp)
    bluecherry_add_test (RangeMapTestCase tests/src/utils/RangeMapTestCase.cpp)
    bluecherry_add_test (RangeTestCase tests/src/utils/RangeTestCase.cpp)
    bluecherry_add_test (EventParserTestCase tests/src/event/EventParserTestCase.cpp)
endif (NOT APPLE)
