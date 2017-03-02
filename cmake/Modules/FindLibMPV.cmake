#
# Copyright 2010-2017 Bluecherry
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

if (NOT WIN32 AND NOT LIBMPV_INCLUDE_DIRS)
    find_package (PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules (LIBMPV QUIET libmpv)
    endif (PKG_CONFIG_FOUND)
endif ()

find_path (LIBMPV_INCLUDE_DIR mpv/client.h ${LIBMPV_INCLUDE_DIRS} ${WIN32_LIBMPV_DIR}/include)
list (APPEND LIBMPV_INCLUDE_DIRS ${LIBMPV_INCLUDE_DIR})
find_library (LIBMPV_LIBRARY NAMES mpv HINTS ${LIBMPV_LIBDIR} ${LIBMPV_LIBRARY_DIRS} ${WIN32_LIBMPV_DIR}/bin)
list (APPEND LIBMPV_LIBRARIES ${LIBMPV_LIBRARY})

#if (UNIX AND NOT APPLE)
#	list (APPEND LIBMPV_LIBRARIES "-ldl" "-lpthread" "-lasound" "-lm")
#endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (LibMPV DEFAULT_MSG LIBMPV_LIBRARIES LIBMPV_INCLUDE_DIRS)
set (LIBMPV_FOUND ${LibMPV_FOUND})

if (LIBMPV_FOUND)
    set (LIBMPV_LIBRARIES ${LIBMPV_LIBRARY})
endif (LIBMPV_FOUND)


mark_as_advanced (LIBMPV_INCLUDE_DIRS LIBMPV_LIBRARIES)

