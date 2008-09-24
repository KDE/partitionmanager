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

FILE(GLOB partitionmanager_SRCS
	core/*.cpp
	gui/*.cpp
	util/*.cpp
	ops/*.cpp
	jobs/*.cpp
	fs/*.cpp
	main.cpp
)

FILE(GLOB partitionmanager_UIFILES gui/*.ui)

KDE4_ADD_UI_FILES(partitionmanager_SRCS ${partitionmanager_UIFILES})

KDE4_ADD_KCFG_FILES(partitionmanager_SRCS config.kcfgc)

KDE4_ADD_EXECUTABLE(partitionmanager ${partitionmanager_SRCS})

TARGET_LINK_LIBRARIES(partitionmanager ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${LIBPARTED_LIBS} ${UUID_LIBS})

INSTALL(TARGETS partitionmanager DESTINATION ${BIN_INSTALL_DIR})
INSTALL(FILES gui/partitionmanagerui.rc DESTINATION ${DATA_INSTALL_DIR}/partitionmanager)
INSTALL(FILES partitionmanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

