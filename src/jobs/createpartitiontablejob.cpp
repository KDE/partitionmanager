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

#include "jobs/createpartitiontablejob.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include "util/report.h"

#include <klocale.h>

#include <parted/parted.h>

/** Creates a new CreatePartitionTableJob
	@param d the Device a new PartitionTable is to be created on
*/
CreatePartitionTableJob::CreatePartitionTableJob(Device& d) :
	Job(),
	m_Device(d)
{
}

bool CreatePartitionTableJob::run(Report& parent)
{
	bool rval = false;

	Report* report = jobStarted(parent);

	if (openPed(device().deviceNode(), true))
	{
		PedDiskType* pedDiskType = ped_disk_type_get(device().partitionTable().typeName().toAscii());

		if (pedDiskType)
		{
			PedDisk* pedDisk = ped_disk_new_fresh(pedDevice(), pedDiskType);
			rval = commit(pedDisk);
			ped_disk_destroy(pedDisk);
		}
		else
			report->line() << i18nc("@info/plain", "Creating partition table failed: Could not retrieve partition table type \"%1\" for <filename>%2</filename>.", device().partitionTable().typeName(), device().deviceNode());

		closePed();
	}
	else
		report->line() << i18nc("@info/plain", "Creating partition table failed: Could not open device <filename>%1</filename>.", device().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString CreatePartitionTableJob::description() const
{
	return i18nc("@info/plain", "Create new partition table on device <filename>%1</filename>", device().deviceNode());
}
