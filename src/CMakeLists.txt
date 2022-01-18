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
	KF5::ConfigCore
	KF5::ConfigGui
	KF5::ConfigWidgets
	KF5::CoreAddons
	KF5::Crash
	KF5::DBusAddons
	KF5::I18n
	KF5::JobWidgets
	KF5::KIOWidgets
	KF5::WidgetsAddons
	KF5::XmlGui
	PolkitQt${QT_MAJOR_VERSION}-1::Core
)

target_compile_definitions(partitionmanager PRIVATE -DCMAKE_INSTALL_FULL_LIBEXECDIR_KF5=\"${CMAKE_INSTALL_FULL_LIBEXECDIR_KF5}\")

install(TARGETS partitionmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.partitionmanager.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES gui/partitionmanagerui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/partitionmanager)
install(PROGRAMS org.kde.partitionmanager.desktop DESTINATION ${KDE_INSTALL_APPDIR})

############################################

install(FILES open_in_partitionmanager.desktop
    open_in_partitionmanager.desktop
    DESTINATION ${KDE_INSTALL_DATADIR}/solid/actions/)
