# SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2014-2022 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "04")
set (RELEASE_SERVICE_VERSION_MICRO "1")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(partitionmanager VERSION ${RELEASE_SERVICE_VERSION})

# Dependencies
set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "5.240.0")
set(KPMCORE_MIN_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}")

set(KDE_COMPILERSETTINGS_LEVEL "5.85")

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(ECMInstallIcons)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMDeprecationSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
  Core
  Gui
  Widgets
)

# Load the frameworks we need
find_package(KF6 ${KF_MIN_VERSION} REQUIRED
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
    WindowSystem
    XmlGui
  OPTIONAL_COMPONENTS
    DocTools
)

find_package(PolkitQt6-1 REQUIRED)
ecm_set_disabled_deprecation_versions(QT 6.5
    KF 5.240.0
)


find_package(KPMcore ${KPMCORE_MIN_VERSION} REQUIRED)

add_subdirectory(src)
add_subdirectory(icons)
if (KF6DocTools_FOUND)
    add_subdirectory(doc)
    kdoctools_install(po)
endif()

ki18n_install(po)

message(STATUS "KDE Partition Manager ${RELEASE_SERVICE_VERSION} will be built for install into ${CMAKE_INSTALL_PREFIX}")

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
