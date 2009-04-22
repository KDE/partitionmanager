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

project(partitionmanager)

cmake_minimum_required(VERSION 2.6.2)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_RELEASE "0")
set(VERSION_SUFFIX "-BETA1")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}${VERSION_SUFFIX})
add_definitions(-D'VERSION="${VERSION}"')

set(KDE_MIN_VERSION 4.2)
find_package(KDE4 REQUIRED)

find_package(MSGFMT REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(UUID REQUIRED uuid)
pkg_check_modules(BLKID REQUIRED blkid)
find_package(LIBPARTED REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(${KDE4_ENABLE_EXCEPTIONS})

include(KDE4Defaults)
include(MacroLibrary)

include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${UUID_INCLUDE_DIRS} ${BLKID_INCLUDE_DIRS} src/)

add_subdirectory(src)
add_subdirectory(icons)

message(STATUS "KDE Partition Manager ${VERSION} will be built for install into ${CMAKE_INSTALL_PREFIX}")

add_custom_target(docs
	COMMAND doxygen Doxyfile
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
