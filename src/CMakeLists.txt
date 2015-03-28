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
include(backend/CMakeLists.txt)
include(core/CMakeLists.txt)
include(util/CMakeLists.txt)
include(ops/CMakeLists.txt)
include(jobs/CMakeLists.txt)
include(fs/CMakeLists.txt)
include(gui/CMakeLists.txt)

set(kpmcore_SRCS
	${BACKEND_SRC}
	${FS_SRC}
	${CORE_SRC}
	${OPS_SRC}
	${JOBS_SRC}
	${UTIL_SRC}
)

set(kpmgui_SRCS
	${CONFIG_SRC}
	${GUI_SRC}
	${UTILGUI_SRC}
)

file(GLOB kpmcore_UIFILES config/*.ui)
file(GLOB kpmgui_UIFILES gui/*.ui)

ki18n_wrap_ui(kpmcore_SRCS ${kpmcore_UIFILES})
ki18n_wrap_ui(kpmgui_SRCS ${kpmgui_UIFILES})

kconfig_add_kcfg_files(kpmcore_SRCS config.kcfgc)
kconfig_add_kcfg_files(kpmgui_SRCS config.kcfgc)
install(FILES config.kcfg DESTINATION ${KCFG_INSTALL_DIR})

add_library(kpmcore SHARED ${kpmcore_SRCS})
generate_export_header(kpmcore EXPORT_FILE_NAME libpartitionmanager_export.h)
target_link_libraries( kpmcore libfatlabel
	${UUID_LIBRARIES} ${BLKID_LIBRARIES} ${LIBATASMART_LIBRARIES}
	KF5::ConfigCore
	KF5::ConfigGui
	KF5::Crash
	KF5::I18n
	KF5::IconThemes
	KF5::KIOCore
	KF5::Service
)

add_library(kpmgui SHARED ${kpmgui_SRCS})
generate_export_header(kpmgui EXPORT_FILE_NAME libpartitionmanagergui_export.h)
target_link_libraries( kpmgui
	kpmcore
	KF5::JobWidgets
	KF5::KIOWidgets
	KF5::WidgetsAddons
	KF5::XmlGui
)

install(TARGETS kpmcore kpmgui ${INSTALL_TARGETS_DEFAULT_ARGS})

############################################

add_executable(partitionmanager main.cpp)

target_link_libraries(partitionmanager kpmcore kpmgui)

install(TARGETS partitionmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.PartitionManager.appdata.xml DESTINATION share/appdata)
install(FILES gui/partitionmanagerui.rc DESTINATION ${DATA_INSTALL_DIR}/partitionmanager)
install(PROGRAMS org.kde.PartitionManager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

############################################

add_subdirectory(plugins)
