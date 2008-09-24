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

#include "jobs/deletepartitionjob.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/report.h"

#include <klocale.h>
#include <kdebug.h>

#include <parted/parted.h>

/** Creates a new DeletePartitionJob
	@param d the Device the Partition to delete is on
	@param p the Partition to delete
*/
DeletePartitionJob::DeletePartitionJob(Device& d, Partition& p) :
	Job(),
	m_Device(d),
	m_Partition(p)
{
}

bool DeletePartitionJob::run(Report& parent)
{
	Q_ASSERT(device().deviceNode() == partition().devicePath());
	
	if (device().deviceNode() != partition().devicePath())
	{
		kWarning() << "deviceNode: " << device().deviceNode() << ", partition path: " << partition().devicePath();
		return false;
	}

	bool rval = false;

	Report* report = jobStarted(parent);
	
	if (openPed(device().deviceNode()))
	{
		PedPartition* pedPartition = partition().roles().has(PartitionRole::Extended)
				? ped_disk_extended_partition(pedDisk())
				: ped_disk_get_partition_by_sector(pedDisk(), partition().firstSector());

		if (pedPartition)
		{
			rval = ped_disk_delete_partition(pedDisk(), pedPartition) && commit();

			if (!rval)
				report->line() << i18nc("@info/plain", "Could not delete partition <filename>%1</filename>.", partition().deviceNode());
		}
		else
			report->line() << i18nc("@info/plain", "Deleting partition failed: Partition to delete (<filename>%1</filename>) not found on disk.", partition().deviceNode());

		closePed();
	}
	else
		report->line() << i18nc("@info/plain", "Deleting partition failed: Could not open device <filename>%1</filename>.", device().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString DeletePartitionJob::description() const
{
	return i18nc("@info/plain", "Delete the partition <filename>%1</filename>", partition().deviceNode());
}
