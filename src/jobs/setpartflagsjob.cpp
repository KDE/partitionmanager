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

#include "jobs/setpartflagsjob.h"

#include "core/device.h"
#include "core/partition.h"
#include "core/partitionrole.h"
#include "core/partitiontable.h"

#include "util/report.h"

#include <klocale.h>

static const struct
{
	PedPartitionFlag pedFlag;
	PartitionTable::Flag flag;
} flagmap[] =
{
	{ PED_PARTITION_BOOT, PartitionTable::FlagBoot },
	{ PED_PARTITION_ROOT, PartitionTable::FlagRoot },
	{ PED_PARTITION_SWAP, PartitionTable::FlagSwap },
	{ PED_PARTITION_HIDDEN, PartitionTable::FlagHidden },
	{ PED_PARTITION_RAID, PartitionTable::FlagRaid },
	{ PED_PARTITION_LVM, PartitionTable::FlagLvm },
	{ PED_PARTITION_LBA, PartitionTable::FlagLba },
	{ PED_PARTITION_HPSERVICE, PartitionTable::FlagHpService },
	{ PED_PARTITION_PALO, PartitionTable::FlagPalo },
	{ PED_PARTITION_PREP, PartitionTable::FlagPrep },
	{ PED_PARTITION_MSFT_RESERVED, PartitionTable::FlagMsftReserved }
};

/** Creates a new SetPartFlagsJob
	@param d the Device the Partition whose flags are to be set is on
	@param p the Partition whose flags are to be set
	@param flags the new flags for the Partition
*/
SetPartFlagsJob::SetPartFlagsJob(Device& d, Partition& p, PartitionTable::Flags flags) :
	Job(),
	m_Device(d),
	m_Partition(p),
	m_Flags(flags)
{
}

qint32 SetPartFlagsJob::numSteps() const
{
	return sizeof(flagmap) / sizeof(flagmap[0]);
}

bool SetPartFlagsJob::run(Report& parent)
{
	bool rval = true;

	Report* report = jobStarted(parent);
	
	if (openPed(device().deviceNode()))
	{
		PedPartition* pedPartition = (partition().roles().has(PartitionRole::Extended)) ? ped_disk_extended_partition(pedDisk()) : ped_disk_get_partition_by_sector(pedDisk(), partition().firstSector());

		if (pedPartition)
		{
			for (quint32 i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++)
			{
				emit progress(i + 1);

				if (!ped_partition_is_flag_available(pedPartition, flagmap[i].pedFlag))
				{
					report->line() << i18nc("@info/plain", "The flag \"%1\" is not available on the partition's partition table.", PartitionTable::flagName(flagmap[i].flag));
					continue;
				}

				// Workaround: libparted claims the hidden flag is available for extended partitions, but
				// throws an error when we try to set or clear it. So skip this combination (also see below in
				// availableFlags()).
				if (pedPartition->type == PED_PARTITION_EXTENDED && flagmap[i].flag == PartitionTable::FlagHidden)
					continue;

				int state = (flags() & flagmap[i].flag) ? 1 : 0;

				if (!ped_partition_set_flag(pedPartition, flagmap[i].pedFlag, state))
				{
					report->line() << i18nc("@info/plain", "There was an error setting flag %1 for partition <filename>%2</filename> to state %3.", PartitionTable::flagName(flagmap[i].flag), partition().deviceNode(), state ? i18nc("@info/plain flag turned on, active", "on") : i18nc("@info/plain flag turned off, inactive", "off"));

					rval = false;
				}
			}
	
			if (!commit())
				rval = false;
		}
		else
			report->line() << i18nc("@info/plain", "Could not find partition <filename>%1</filename> on device <filename>%2</filename> to set partition flags.", partition().deviceNode(), device().deviceNode());
	
		closePed();
	}
	else
		report->line() << i18nc("@info/plain", "Could not open device <filename>%1</filename> to set partition flags for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
	
	if (rval)
		partition().setFlags(flags());

	jobFinished(*report, rval);

	return rval;
}

PartitionTable::Flags SetPartFlagsJob::activeFlags(PedPartition* p)
{
	PartitionTable::Flags flags = PartitionTable::FlagNone;

	// We might get here with a pedPartition just picked up from libparted that is
	// unallocated. Libparted doesn't like it if we ask for flags for unallocated
	// space.
	if (p->num <= 0)
		return flags;
	
	for (quint32 i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++)
		if (ped_partition_is_flag_available(p, flagmap[i].pedFlag) && ped_partition_get_flag(p, flagmap[i].pedFlag))
			flags |= flagmap[i].flag;

	return flags;
}

PartitionTable::Flags SetPartFlagsJob::availableFlags(PedPartition* p)
{
	PartitionTable::Flags flags;
	
	// see above.
	if (p->num <= 0)
		return flags;
	
	for (quint32 i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++)
		if (ped_partition_is_flag_available(p, flagmap[i].pedFlag))
		{
			// workaround:: see above
			if (p->type != PED_PARTITION_EXTENDED || flagmap[i].flag != PartitionTable::FlagHidden)
				flags |= flagmap[i].flag;
		}
	
	return flags;
}

QString SetPartFlagsJob::description() const
{
	if (PartitionTable::flagNames(flags()).size() == 0)
		return QString(i18nc("@info/plain", "Clear flags for partition <filename>%1</filename>", partition().deviceNode()));

	return i18nc("@info/plain", "Set the flags for partition <filename>%1</filename> to \"%2\"", partition().deviceNode(), PartitionTable::flagNames(flags()).join(","));
}
