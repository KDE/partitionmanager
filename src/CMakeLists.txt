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

FILE(GLOB partitionmanagerprivate_SRCS
	core/*.cpp
	util/*.cpp
	ops/*.cpp
	jobs/*.cpp
	fs/*.cpp
	gui/*.cpp
)

FILE(GLOB partitionmanagerprivate_UIFILES gui/*.ui)

KDE4_ADD_UI_FILES(partitionmanagerprivate_SRCS ${partitionmanagerprivate_UIFILES})

KDE4_ADD_KCFG_FILES(partitionmanagerprivate_SRCS config.kcfgc)

KDE4_ADD_LIBRARY(partitionmanagerprivate SHARED ${partitionmanagerprivate_SRCS})

TARGET_LINK_LIBRARIES(partitionmanagerprivate ${KDE4_KDECORE_LIBS} ${KDE4_KFILE_LIBS} ${LIBPARTED_LIBS} ${UUID_LIBRARIES} ${BLKID_LIBRARIES})

INSTALL(TARGETS partitionmanagerprivate DESTINATION ${LIB_INSTALL_DIR})

############################################

FILE(GLOB partitionmanager_SRCS	main.cpp)

KDE4_ADD_EXECUTABLE(partitionmanager ${partitionmanager_SRCS})

TARGET_LINK_LIBRARIES(partitionmanager partitionmanagerprivate)

INSTALL(TARGETS partitionmanager DESTINATION ${BIN_INSTALL_DIR})
INSTALL(FILES gui/partitionmanagerui.rc DESTINATION ${DATA_INSTALL_DIR}/partitionmanager)
INSTALL(FILES partitionmanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

############################################

OPTION(PARTMAN_KPART "Build a kpart for KDE Partition Manager" OFF)

IF(PARTMAN_KPART)
	FILE(GLOB partitionmanagerpart_SRCS kpart/partitionmanagerpart.cpp)

	KDE4_ADD_PLUGIN(partitionmanagerpart ${partitionmanagerpart_SRCS})

	TARGET_LINK_LIBRARIES(partitionmanagerpart
		${KDE4_KPARTS_LIBS}
		${KDE4_KDECORE_LIBS}
		partitionmanagerprivate
	)

	INSTALL(TARGETS partitionmanagerpart DESTINATION ${PLUGIN_INSTALL_DIR})
	INSTALL(FILES kpart/partitionmanagerpart.desktop DESTINATION ${SERVICES_INSTALL_DIR})
	INSTALL(FILES kpart/partitionmanagerpart.rc DESTINATION ${DATA_INSTALL_DIR}/partitionmanagerpart)

	OPTION(PARTMAN_KPART_TEST "Build a kpart test application for KDE Partition Manager" OFF)

	IF(PARTMAN_KPART_TEST)
		ADD_SUBDIRECTORY(kpart/test)
	ENDIF(PARTMAN_KPART_TEST)

ENDIF(PARTMAN_KPART)

