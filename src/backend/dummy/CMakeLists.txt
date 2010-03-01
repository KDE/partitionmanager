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

file (GLOB pluginpmdummy_SRCS *.cpp)

kde4_add_library(pluginpmdummy SHARED ${pluginpmdummy_SRCS})

target_link_libraries(pluginpmdummy partitionmanagerprivate ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${LIBPARTED_LIBS} ${BLKID_LIBRARIES} ${KDE4_SOLID_LIBS})

set_target_properties(pluginpmdummy PROPERTIES PREFIX "")

install(TARGETS pluginpmdummy DESTINATION ${PLUGIN_INSTALL_DIR})
