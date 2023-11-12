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
	KF6::ConfigCore
	KF6::ConfigGui
	KF6::ConfigWidgets
	KF6::CoreAddons
	KF6::Crash
	KF6::DBusAddons
	KF6::I18n
	KF6::JobWidgets
	KF6::KIOWidgets
	KF6::WidgetsAddons
	KF6::XmlGui
	KF6::WindowSystem
	PolkitQt6-1::Core
)

target_compile_definitions(partitionmanager PRIVATE -DCMAKE_INSTALL_FULL_LIBEXECDIR_KF6=\"${CMAKE_INSTALL_FULL_LIBEXECDIR_KF6}\")

install(TARGETS partitionmanager ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.partitionmanager.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(PROGRAMS org.kde.partitionmanager.desktop DESTINATION ${KDE_INSTALL_APPDIR})

############################################

install(FILES open_in_partitionmanager.desktop
    open_in_partitionmanager.desktop
    DESTINATION ${KDE_INSTALL_DATADIR}/solid/actions/)
