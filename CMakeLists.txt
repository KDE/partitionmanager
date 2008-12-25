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

PROJECT(partitionmanager)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

SET(CMAKE_USE_RELATIVE_PATHS OFF)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "0")
SET(VERSION_RELEASE "0")
SET(VERSION_SUFFIX "-ALPHA2")
SET(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}${VERSION_SUFFIX})
ADD_DEFINITIONS(-D'VERSION="${VERSION}"')

FIND_PACKAGE(KDE4 REQUIRED)

FIND_PACKAGE(MSGFMT REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(UUID REQUIRED uuid)
PKG_CHECK_MODULES(BLKID REQUIRED blkid)
FIND_PACKAGE(LIBPARTED REQUIRED)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})

INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)

INCLUDE_DIRECTORIES(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${UUID_INCLUDE_DIRS} ${BLKID_INCLUDE_DIRS} src/)

ADD_SUBDIRECTORY(src)

MESSAGE(STATUS "KDE Partition Manager ${VERSION} will be built for install into ${CMAKE_INSTALL_PREFIX}")

ADD_CUSTOM_TARGET(docs
	COMMAND doxygen Doxyfile
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
