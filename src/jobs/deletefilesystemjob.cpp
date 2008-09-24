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

#include "jobs/deletefilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/report.h"

#include <klocale.h>
#include <kdebug.h>

#include <parted/parted.h>

/** Creates a new DeleteFileSystemJob
	@param d the Device the FileSystem to delete is on
	@param p the Partition the FileSystem to delete is on
*/
DeleteFileSystemJob::DeleteFileSystemJob(Device& d, Partition& p) :
	Job(),
	m_Device(d),
	m_Partition(p)
{
}

bool DeleteFileSystemJob::run(Report& parent)
{
	Q_ASSERT(device().deviceNode() == partition().devicePath());
	
	if (device().deviceNode() != partition().devicePath())
	{
		kWarning() << "deviceNode: " << device().deviceNode() << ", partition path: " << partition().devicePath();
		return false;
	}
	
	bool rval = false;

	Report* report = jobStarted(parent);
	
	if (partition().roles().has(PartitionRole::Extended))
		rval = true;
	else if (openPed(device().deviceNode()))
	{
		if (PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk(), partition().firstSector()))
		{
			if (ped_file_system_clobber(&pedPartition->geom))
			{
				if (pedPartition->type == PED_PARTITION_NORMAL || pedPartition->type == PED_PARTITION_LOGICAL)
				{
					if (ped_device_open(pedDevice()))
					{
						// libparted doesn't deal with reiser4, so we overwrite it ourselves here
						rval = ped_geometry_write(&pedPartition->geom, "0000000", 128, 1);

						if (!rval)
							report->line() << i18nc("@info/plain", "Failed to erase reiser4 signature on partition <filename>%1</filename>.", partition().deviceNode());

						ped_device_close(pedDevice());
					}
				}
				else
					rval = true;
			}
			else
				report->line() << i18nc("@info/plain", "Failed to clobber file system on partition <filename>%1</filename>.", partition().deviceNode());
		}
		else
			report->line() << i18nc("@info/plain", "Could not delete file system on partition <filename>%1</filename>: Failed to get partition.", partition().deviceNode());

		closePed();
	}
	else
		report->line() << i18nc("@info/plain", "Could not delete file system signature for partition <filename>%1</filename>: Failed to open device <filename>%2</filename>.", partition().deviceNode(), device().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString DeleteFileSystemJob::description() const
{
	return i18nc("@info/plain", "Delete file system on <filename>%1</filename>", partition().deviceNode());
}
