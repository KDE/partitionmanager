# Copyright (C) 2008, 2012 by Volker Lanz <vl@fidra.de>
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

############################################

include(config/CMakeLists.txt)
include(gui/CMakeLists.txt)
include(util/CMakeLists.txt)

set(partitionmanager_SRCS
	main.cpp
	${CONFIG_SRC}
	${GUI_SRC}
	${UTIL_SRC}
)

file(GLOB partitionmanager_UIFILES config/*.ui gui/*.ui)

ki18n_wrap_ui(partitionmanager_SRCS ${partitionmanager_UIFILES})

kconfig_add_kcfg_files(partitionmanager_SRCS config.kcfgc)
install(FILES partitionmanager.kcfg DESTINATION ${KCFG_INSTALL_DIR})

############################################

add_executable(partitionmanager ${partitionmanager_SRCS} )

target_link_libraries(partitionmanager ${BLKID_LIBRARIES}
	kpmcore
	KF5::ConfigCore
	KF5::ConfigGui
	KF5::ConfigWidgets
	KF5::CoreAddons
	KF5::Crash
	KF5::DBusAddons
	KF5::I18n
	KF5::IconThemes
	KF5::JobWidgets
	KF5::KIOWidgets
	KF5::WidgetsAddons
	KF5::XmlGui
)

target_compile_definitions(partitionmanager PRIVATE -DCMAKE_INSTALL_FULL_LIBEXECDIR_KF5=\"${CMAKE_INSTALL_FULL_LIBEXECDIR_KF5}\")

install(TARGETS partitionmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.partitionmanager.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES gui/partitionmanagerui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/partitionmanager)
install(PROGRAMS org.kde.partitionmanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

############################################
