# SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

project(partitionmanager)

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Dependencies
set(QT_MIN_VERSION "5.10.0")
set(KF5_MIN_VERSION "5.71")
set(KPMCORE_MIN_VERSION "4.1.0")

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(KDE_INSTALL_USE_QT_SYS_PATHS ON CACHE BOOL "Install mkspecs files, Plugins and Imports to the Qt 5 install dir" FORCE)

# Check KPMcore dependency when bumping
set(VERSION_MAJOR "4")
set(VERSION_MINOR "1")
set(VERSION_RELEASE "0")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE})
add_definitions(-D'VERSION="${VERSION}"') #"

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
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
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED
  COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    I18n
    IconThemes
    JobWidgets
    KIO
    WidgetsAddons
    XmlGui
  OPTIONAL_COMPONENTS
    DocTools
)

# use sane compile flags
add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_CAST_TO_BYTEARRAY
  -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_DISABLE_DEPRECATED_BEFORE=0x050900
  -DQT_NO_KEYWORDS
)

find_package(KPMcore ${KPMCORE_MIN_VERSION} REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS} ${UUID_INCLUDE_DIRS} ${BLKID_INCLUDE_DIRS} ${KPMCORE_INCLUDE_DIR} lib/ src/)

add_subdirectory(src)
add_subdirectory(icons)
if (KF5DocTools_FOUND)
    add_subdirectory(doc)
endif()

ki18n_install(po)

message(STATUS "KDE Partition Manager ${VERSION} will be built for install into ${CMAKE_INSTALL_PREFIX}")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
