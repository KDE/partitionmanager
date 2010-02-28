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

############################################

file(GLOB partitionmanagerprivate_SRCS
	backend/*.cpp
	backend/libparted/*.cpp
	core/*.cpp
	util/*.cpp
	ops/*.cpp
	jobs/*.cpp
	fs/*.cpp
	gui/*.cpp
)

file(GLOB partitionmanagerprivate_UIFILES gui/*.ui kcm/*.ui)

kde4_add_ui_files(partitionmanagerprivate_SRCS ${partitionmanagerprivate_UIFILES})

kde4_add_kcfg_files(partitionmanagerprivate_SRCS config.kcfgc)

kde4_add_library(partitionmanagerprivate SHARED ${partitionmanagerprivate_SRCS})

target_link_libraries(partitionmanagerprivate libfatlabel ${KDE4_KDECORE_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KIO_LIBS} ${LIBPARTED_LIBS} ${UUID_LIBRARIES} ${BLKID_LIBRARIES} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS partitionmanagerprivate ${INSTALL_TARGETS_DEFAULT_ARGS})

############################################

file(GLOB partitionmanager-bin_SRCS	main.cpp)

kde4_add_app_icon(partitionmanager-bin_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-apps-partitionmanager.png")

kde4_add_executable(partitionmanager-bin ${partitionmanager-bin_SRCS})

target_link_libraries(partitionmanager-bin partitionmanagerprivate ${KDE4_KDEUI_LIBS})

if(UNIX)
	add_custom_target(partitionmanager ALL
		COMMAND sed -e 's,@INSTALL_PATH@,${BIN_INSTALL_DIR},g' < ${CMAKE_CURRENT_SOURCE_DIR}/partitionmanager.in > ${CMAKE_CURRENT_BINARY_DIR}/partitionmanager
	)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/partitionmanager DESTINATION ${BIN_INSTALL_DIR})
endif(UNIX)
install(TARGETS partitionmanager-bin ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES gui/partitionmanagerui.rc DESTINATION ${DATA_INSTALL_DIR}/partitionmanager)
install(FILES partitionmanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

############################################

option(PARTMAN_KPART "Build a kpart for KDE Partition Manager" OFF)

if(PARTMAN_KPART)
	file(GLOB partitionmanagerpart_SRCS kpart/partitionmanagerpart.cpp)

	kde4_add_plugin(partitionmanagerpart ${partitionmanagerpart_SRCS})

	target_link_libraries(partitionmanagerpart
		${KDE4_KPARTS_LIBS}
		${KDE4_KDECORE_LIBS}
		partitionmanagerprivate
	)

	install(TARGETS partitionmanagerpart DESTINATION ${PLUGIN_INSTALL_DIR})
	install(FILES kpart/partitionmanagerpart.desktop DESTINATION ${SERVICES_INSTALL_DIR})
	install(FILES kpart/partitionmanagerpart.rc DESTINATION ${DATA_INSTALL_DIR}/partitionmanagerpart)

	option(PARTMAN_KPART_TEST "Build a kpart test application for KDE Partition Manager" OFF)

	if(PARTMAN_KPART_TEST)
		add_subdirectory(kpart/test)
	endif(PARTMAN_KPART_TEST)

endif(PARTMAN_KPART)

