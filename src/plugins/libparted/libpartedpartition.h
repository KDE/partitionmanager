/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#if !defined(LIBPARTEDPARTITION__H)

#define LIBPARTEDPARTITION__H

#include "backend/corebackendpartition.h"

#include "core/partitiontable.h"

#include <parted/parted.h>

class Report;

class LibPartedPartition : public CoreBackendPartition
{
	Q_DISABLE_COPY(LibPartedPartition);

	public:
		LibPartedPartition(PedPartition* ped_partition);

	public:
		virtual bool setFlag(Report& report, PartitionTable::Flag flag, bool state);

	private:
		PedPartition* pedPartition() { return m_PedPartition; }

	private:
		PedPartition* m_PedPartition;
};


#endif
