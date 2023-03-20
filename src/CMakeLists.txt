# SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

include(config/CMakeLists.txt)
include(gui/CMakeLists.txt)
include(util/CMakeLists.txt)

set_property(SOURCE main.cpp APPEND PROPERTY COMPILE_DEFINITIONS "VERSION=\"${RELEASE_SERVICE_VERSION}\"")

set(partitionmanager_SRCS
	main.cpp
	${CONFIG_SRC}
	${GUI_SRC}
	${UTIL_SRC}
)

file(GLOB partitionmanager_UIFILES config/*.ui gui/*.ui)

ki18n_wrap_ui(partitionmanager_SRCS ${partitionmanager_UIFILES})

kconfig_add_kcfg_files(partitionmanager_SRCS config.kcfgc)
install(FILES partitionmanager.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

############################################

add_executable(partitionmanager ${partitionmanager_SRCS} )

target_link_libraries(partitionmanager
	kpmcore
	KF${KF_MAJOR_VERSION}::ConfigCore
	KF${KF_MAJOR_VERSION}::ConfigGui
	KF${KF_MAJOR_VERSION}::ConfigWidgets
	KF${KF_MAJOR_VERSION}::CoreAddons
	KF${KF_MAJOR_VERSION}::Crash
	KF${KF_MAJOR_VERSION}::DBusAddons
	KF${KF_MAJOR_VERSION}::I18n
	KF${KF_MAJOR_VERSION}::JobWidgets
	KF${KF_MAJOR_VERSION}::KIOWidgets
	KF${KF_MAJOR_VERSION}::WidgetsAddons
	KF${KF_MAJOR_VERSION}::XmlGui
	KF${KF_MAJOR_VERSION}::WindowSystem
	PolkitQt${QT_MAJOR_VERSION}-1::Core
)

target_compile_definitions(partitionmanager PRIVATE -DCMAKE_INSTALL_FULL_LIBEXECDIR_KF5=\"${CMAKE_INSTALL_FULL_LIBEXECDIR_KF5}\")

install(TARGETS partitionmanager ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.partitionmanager.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES gui/partitionmanagerui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/partitionmanager)
install(PROGRAMS org.kde.partitionmanager.desktop DESTINATION ${KDE_INSTALL_APPDIR})

############################################

install(FILES open_in_partitionmanager.desktop
    open_in_partitionmanager.desktop
    DESTINATION ${KDE_INSTALL_DATADIR}/solid/actions/)
