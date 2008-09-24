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

#include "jobs/setpartgeometryjob.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/report.h"

#include <kdebug.h>
#include <klocale.h>

#include <parted/parted.h>

/** Creates a new SetPartGeometryJob
	@param d the Device the Partition whose geometry is to be set is on
	@param p the Partition whose geometry is to be set
	@param newstart the new start sector for the Partition
	@param newlength the new length for the Partition

	@todo Wouldn't it be better to have newfirst (new first sector) and newlast (new last sector) as args instead?
	Having a length here doesn't seem to be very consistent with the rest of the app, right?
*/
SetPartGeometryJob::SetPartGeometryJob(Device& d, Partition& p, qint64 newstart, qint64 newlength) :
	Job(),
	m_Device(d),
	m_Partition(p),
	m_NewStart(newstart),
	m_NewLength(newlength)
{
}

bool SetPartGeometryJob::run(Report& parent)
{
	bool rval = false;

	Report* report = jobStarted(parent);

	if (openPed(device().deviceNode()))
	{
		PedPartition* pedPartition = (partition().roles().has(PartitionRole::Extended)) ? ped_disk_extended_partition(pedDisk()) : ped_disk_get_partition_by_sector(pedDisk(), partition().firstSector());

		if (pedPartition)
		{
			if (PedGeometry* pedGeometry = ped_geometry_new(pedDevice(), newStart(), newLength()))
			{
				if (PedConstraint* pedConstraint = ped_constraint_exact(pedGeometry))
				{
					if (ped_disk_set_partition_geom(pedDisk(), pedPartition, pedConstraint, newStart(), newStart() + newLength() - 1) && commit())
					{
						rval = true;
						partition().setFirstSector(pedPartition->geom.start);
						partition().setLastSector(pedPartition->geom.end);
					}
					else
						report->line() << i18nc("@info/plain", "Could not set geometry for partition <filename>%1</filename> while trying to resize/move it.", partition().deviceNode());
				}
				else
					report->line() << i18nc("@info/plain", "Could not get constraint for partition <filename>%1</filename> while trying to resize/move it.", partition().deviceNode());
			}
			else
				report->line() << i18nc("@info/plain", "Could not get geometry for partition <filename>%1</filename> while trying to resize/move it.", partition().deviceNode());
		}
		else
			report->line() << i18nc("@info/plain", "Could not open partition <filename>%1</filename> while trying to resize/move it.", partition().deviceNode());
	
		closePed();
	}
	else
		report->line() << i18nc("@info/plain", "Could not open device <filename>%1</filename> while trying to resize/move partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString SetPartGeometryJob::description() const
{
	return i18nc("@info/plain", "Set geometry of partition <filename>%1</filename>: Start sector: %2, length: %3", partition().deviceNode(), newStart(), newLength());
}
