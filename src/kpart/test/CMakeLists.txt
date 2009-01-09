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

find_package(KDE4 REQUIRED)

include_directories(${KDE4_INCLUDES} .)

set(kparttest_SRCS
	main.cpp
	kparttest.cpp
)

kde4_add_executable(kparttest ${kparttest_SRCS})

target_link_libraries(kparttest ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kparttest DESTINATION ${BIN_INSTALL_DIR})
install(FILES kparttestui.rc DESTINATION  ${DATA_INSTALL_DIR}/kparttest)
