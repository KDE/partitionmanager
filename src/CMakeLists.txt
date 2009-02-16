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
	core/*.cpp
	util/*.cpp
	ops/*.cpp
	jobs/*.cpp
	fs/*.cpp
	gui/*.cpp
)

file(GLOB partitionmanagerprivate_UIFILES gui/*.ui)

kde4_add_ui_files(partitionmanagerprivate_SRCS ${partitionmanagerprivate_UIFILES})

kde4_add_kcfg_files(partitionmanagerprivate_SRCS config.kcfgc)

kde4_add_library(partitionmanagerprivate SHARED ${partitionmanagerprivate_SRCS})

target_link_libraries(partitionmanagerprivate ${KDE4_KDECORE_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KIO_LIBS} ${LIBPARTED_LIBS} ${UUID_LIBRARIES} ${BLKID_LIBRARIES})

install(TARGETS partitionmanagerprivate ${INSTALL_TARGETS_DEFAULT_ARGS})

############################################

file(GLOB partitionmanager_SRCS	main.cpp)

kde4_add_executable(partitionmanager ${partitionmanager_SRCS})

target_link_libraries(partitionmanager partitionmanagerprivate)

install(TARGETS partitionmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
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

