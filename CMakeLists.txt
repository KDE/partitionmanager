# SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2014-2022 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "04")
set (RELEASE_SERVICE_VERSION_MICRO "0")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(partitionmanager VERSION ${RELEASE_SERVICE_VERSION})

# Dependencies
set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.90")
set(KPMCORE_MIN_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}")

set(KDE_COMPILERSETTINGS_LEVEL "5.85")

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(ECMInstallIcons)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(GenerateExportHeader)

if (NOT QT_MAJOR_VERSION)
    set(QT_MAJOR_VERSION "5")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
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
    JobWidgets
    KIO
    WidgetsAddons
    XmlGui
  OPTIONAL_COMPONENTS
    DocTools
)

find_package(PolkitQt${QT_MAJOR_VERSION}-1 REQUIRED)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)

find_package(KPMcore ${KPMCORE_MIN_VERSION} REQUIRED)

add_subdirectory(src)
add_subdirectory(icons)
if (KF5DocTools_FOUND)
    add_subdirectory(doc)
    kdoctools_install(po)
endif()

ki18n_install(po)

message(STATUS "KDE Partition Manager ${RELEASE_SERVICE_VERSION} will be built for install into ${CMAKE_INSTALL_PREFIX}")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
