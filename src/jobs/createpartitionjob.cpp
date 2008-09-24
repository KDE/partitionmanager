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

#include "jobs/createpartitionjob.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/report.h"

#include <klocale.h>

#include <parted/parted.h>

/** Creates a new CreatePartitionJob
	@param d the Device the Partition to be created will be on
	@param p the Partition to create
*/
CreatePartitionJob::CreatePartitionJob(Device& d, Partition& p) :
	Job(),
	m_Device(d),
	m_Partition(p)
{
}

bool CreatePartitionJob::run(Report& parent)
{
	Q_ASSERT(partition().devicePath() == device().deviceNode());

	bool rval = false;

	Report* report = jobStarted(parent);

	// According to libParted docs, PedPartitionType can be "NULL if unknown". That's obviously wrong,
	// it's a typedef for an enum. So let's use something the libparted devs will hopefully never
	// use...
	PedPartitionType pedType = static_cast<PedPartitionType>(0xffffffff);

	if (partition().roles().has(PartitionRole::Extended))
		pedType = PED_PARTITION_EXTENDED;
	else if (partition().roles().has(PartitionRole::Logical))
		pedType = PED_PARTITION_LOGICAL;
	else if (partition().roles().has(PartitionRole::Primary))
		pedType = PED_PARTITION_NORMAL;
			
	if (pedType == static_cast<int>(0xffffffff))
	{
		report->line() << i18nc("@info/plain", "Unknown partition role for new partition <filename>%1</filename> (roles: %2)", partition().deviceNode(), partition().roles().toString());
	}
	else if (openPed(device().deviceNode()))
	{
		PedFileSystemType* pedFsType = (partition().roles().has(PartitionRole::Extended) || partition().fileSystem().type() == FileSystem::Unformatted) ? NULL : getPedFileSystemType(partition().fileSystem().type());

		PedPartition* pedPartition = ped_partition_new(pedDisk(), pedType, pedFsType, partition().firstSector(), partition().lastSector());

		if (pedPartition)
		{
			PedConstraint* pedConstraint = NULL;
			PedGeometry* pedGeometry = ped_geometry_new(pedDevice(), partition().firstSector(), partition().length());

			if (pedGeometry)
				pedConstraint = ped_constraint_exact(pedGeometry);

			if (pedConstraint)
			{
				if (ped_disk_add_partition(pedDisk(), pedPartition, pedConstraint) && commit())
				{
					partition().setNumber(pedPartition->num);
					partition().setState(Partition::StateNone);

					partition().setFirstSector(pedPartition->geom.start);
					partition().setLastSector(pedPartition->geom.end);

					rval = true;
				}
				else
					report->line() << i18nc("@info/plain", "Failed to add partition <filename>%1</filename> to device <filename>%2</filename>.", partition().deviceNode(), device().deviceNode());

				ped_constraint_destroy(pedConstraint);
			}
			else
				report->line() << i18nc("@info/plain", "Failed to create a new partition: could not get geometry for constraint.");
		}
		else
			report->line() << i18nc("@info/plain", "Failed to create new partition <filename>%1</filename>.", partition().deviceNode());
	
		closePed();
	}
	else
		report->line() << i18nc("@info/plain", "Could not open device <filename>%1</filename> to create new partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
	
	jobFinished(*report, rval);
	
	return rval;
}

QString CreatePartitionJob::description() const
{
	if (partition().number() > 0)
		return i18nc("@info/plain", "Create new partition <filename>%1</filename>", partition().deviceNode());

	return i18nc("@info/plain", "Create new partition on device <filename>%1</filename>", device().deviceNode());
}
