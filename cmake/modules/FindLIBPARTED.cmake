# Copyright (C) 2008 by Volker Lanz <vl@fidra.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

FIND_PATH(LIBPARTED_INCLUDE_DIR parted.h PATH_SUFFIXES parted )

FIND_LIBRARY(LIBPARTED_LIBRARY NAMES parted)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBPARTED DEFAULT_MSG LIBPARTED_LIBRARY LIBPARTED_INCLUDE_DIR)

SET(LIBPARTED_LIBS ${LIBPARTED_LIBRARY})

MARK_AS_ADVANCED(LIBPARTED_LIBRARY LIBPARTED_INCLUDE_DIR)
