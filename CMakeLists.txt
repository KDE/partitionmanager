# Copyright (C) 2008 by Volker Lanz <vl@fidra.de>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

project(partitionmanager)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(KDE_INSTALL_USE_QT_SYS_PATHS ON CACHE BOOL "Install mkspecs files, Plugins and Imports to the Qt 5 install dir" FORCE)

set(QT_MIN_VERSION "5.4.1")
# Check KPMcore dependency when bumping
set(VERSION_MAJOR "2")
set(VERSION_MINOR "2")
set(VERSION_RELEASE "0")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE})
add_definitions(-D'VERSION="${VERSION}"') #"

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(ECMInstallIcons)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(GenerateExportHeader)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
  Core
  Gui
  Widgets
)

# Load the frameworks we need
find_package(KF5 5.16 REQUIRED
  Config
  Crash
  DocTools
  I18n
  IconThemes
  JobWidgets
  KIO
  Service
  XmlGui
  WidgetsAddons
)

# use sane compile flags
add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
)

find_package(PkgConfig REQUIRED)
find_package(KPMcore 2.2.0 REQUIRED)
pkg_check_modules(BLKID REQUIRED blkid)

include_directories(${Qt5Core_INCLUDE_DIRS} ${UUID_INCLUDE_DIRS} ${BLKID_INCLUDE_DIRS} ${KPMCORE_INCLUDE_DIR} lib/ src/)

add_subdirectory(src)
add_subdirectory(icons)
add_subdirectory(doc)

ki18n_install(po)

get_filename_component(_doc_translations_path doc-translations ABSOLUTE)

if (EXISTS ${_doc_translations_path}/CMakeLists.txt)
    add_subdirectory(doc-translations)
endif()

message(STATUS "KDE Partition Manager ${VERSION} will be built for install into ${CMAKE_INSTALL_PREFIX}")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
