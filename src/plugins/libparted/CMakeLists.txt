# Copyright (C) 2010 by Volker Lanz <vl@fidra.de>
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

find_package(LIBPARTED REQUIRED)

if (LIBPARTED_FILESYSTEM_SUPPORT)
	add_definitions(-DLIBPARTED_FILESYSTEM_SUPPORT)
endif (LIBPARTED_FILESYSTEM_SUPPORT)

include_directories(${LIBPARTED_INCLUDE_DIR})

file (GLOB pmlibpartedbackendplugin_SRCS *.cpp)

kde4_add_plugin(pmlibpartedbackendplugin ${pmlibpartedbackendplugin_SRCS})

target_link_libraries(pmlibpartedbackendplugin partitionmanagerprivate ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${LIBPARTED_LIBS} ${BLKID_LIBRARIES} ${KDE4_SOLID_LIBS})

install(TARGETS pmlibpartedbackendplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES pmlibpartedbackendplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
