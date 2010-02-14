/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#if !defined(LIBPARTED__H)

#define LIBPARTED__H

#include "core/partitiontable.h"

#include <parted/parted.h>

#include <qglobal.h>

class Device;

class QString;

/** @brief Device scanning done by libparted.

	More libparted-related stuff is in the @link Job Job-derived @endlink classes.

	@author vl@fidra.de
*/
class LibParted
{
	Q_DISABLE_COPY(LibParted)

	public:
		typedef struct
		{
			PedPartitionFlag pedFlag;
			PartitionTable::Flag flag;
		} FlagMap;

	public:
		LibParted();

	public:
		static Device* scanDevice(const QString& device_node);
		static const FlagMap* flagMap();
};

#endif
