/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de                        *
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

#include "plugins/libparted/libpartedpartitiontable.h"
#include "plugins/libparted/libpartedpartition.h"
#include "plugins/libparted/libpartedbackend.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystem.h"

#include "util/report.h"
#include "util/externalcommand.h"

#include <klocale.h>
#include <kdebug.h>

#include <unistd.h>

LibPartedPartitionTable::LibPartedPartitionTable (PedDevice* device) :
	CoreBackendPartitionTable(),
	m_PedDevice(device),
	m_PedDisk(NULL)
{
}

LibPartedPartitionTable::~LibPartedPartitionTable()
{
	ped_disk_destroy(m_PedDisk);
}

bool LibPartedPartitionTable::open()
{
	m_PedDisk = ped_disk_new(pedDevice());

	return m_PedDisk != NULL;
}


bool LibPartedPartitionTable::commit(quint32 timeout)
{
	return commit(pedDisk(), timeout);
}

bool LibPartedPartitionTable::commit(PedDisk* pd, quint32 timeout)
{
	if (pd == NULL)
		return false;

	bool rval = ped_disk_commit_to_dev(pd);

	// The GParted authors have found a bug in libparted that causes it to intermittently
	// not commit changes to the Linux kernel, probably a race. Until this is fixed in
	// libparted, the following patch should help alleviate the consequences by just re-trying
	// committing to the OS if it fails the first time after a short pause.
	// See: http://git.gnome.org/browse/gparted/commit/?id=bf86fd3f9ceb0096dfe87a8c9a38403c13b13f00
	if (rval)
	{
		rval = ped_disk_commit_to_os(pd);

		if (!rval)
		{
			sleep(1);
			rval = ped_disk_commit_to_os(pd);
		}
	}

	if (!ExternalCommand("udevadm", QStringList() << "settle" << "--timeout=" + QString::number(timeout)).run() &&
			!ExternalCommand("udevsettle", QStringList() << "--timeout=" + QString::number(timeout)).run())
		sleep(timeout);

	return rval;
}

CoreBackendPartition* LibPartedPartitionTable::getExtendedPartition()
{
	PedPartition* pedPartition = ped_disk_extended_partition(pedDisk());

	if (pedPartition == NULL)
		return NULL;

	return new LibPartedPartition(pedPartition);
}

CoreBackendPartition* LibPartedPartitionTable::getPartitionBySector(qint64 sector)
{
	PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk(), sector);

	if (pedPartition == NULL)
		return NULL;

	return new LibPartedPartition(pedPartition);
}

static const struct
{
	FileSystem::Type type;
	QString name;
} mapFileSystemTypeToLibPartedName[] =
{
	{ FileSystem::Ext2, "ext2" },
	{ FileSystem::Ext3, "ext3" },
	{ FileSystem::Ext4, "ext4" },
	{ FileSystem::LinuxSwap, "linux-swap" },
	{ FileSystem::Fat16, "fat16" },
	{ FileSystem::Fat32, "fat32" },
	{ FileSystem::Ntfs, "ntfs" },
	{ FileSystem::ReiserFS, "reiserfs" },
	{ FileSystem::Reiser4, "reiser4" },
	{ FileSystem::Xfs, "xfs" },
	{ FileSystem::Jfs, "jfs" },
	{ FileSystem::Hfs, "hfs" },
	{ FileSystem::HfsPlus, "hfs+" },
	{ FileSystem::Ufs, "ufs" }
};

static PedFileSystemType* getPedFileSystemType(FileSystem::Type t)
{
	for (quint32 i = 0; i < sizeof(mapFileSystemTypeToLibPartedName) / sizeof(mapFileSystemTypeToLibPartedName[0]); i++)
		if (mapFileSystemTypeToLibPartedName[i].type == t)
			return ped_file_system_type_get(mapFileSystemTypeToLibPartedName[i].name.toAscii());

	// if we didn't find anything, go with ext2 as a safe fallback
	return ped_file_system_type_get("ext2");
}

qint32 LibPartedPartitionTable::createPartition(Report& report, const Partition& partition)
{
	Q_ASSERT(partition.devicePath() == pedDevice()->path);

	qint32 rval = -1;

	// According to libParted docs, PedPartitionType can be "NULL if unknown". That's obviously wrong,
	// it's a typedef for an enum. So let's use something the libparted devs will hopefully never
	// use...
	PedPartitionType pedType = static_cast<PedPartitionType>(0xffffffff);

	if (partition.roles().has(PartitionRole::Extended))
		pedType = PED_PARTITION_EXTENDED;
	else if (partition.roles().has(PartitionRole::Logical))
		pedType = PED_PARTITION_LOGICAL;
	else if (partition.roles().has(PartitionRole::Primary))
		pedType = PED_PARTITION_NORMAL;

	if (pedType == static_cast<int>(0xffffffff))
	{
		report.line() << i18nc("@info/plain", "Unknown partition role for new partition <filename>%1</filename> (roles: %2)", partition.deviceNode(), partition.roles().toString());
		return false;
	}

	PedFileSystemType* pedFsType = (partition.roles().has(PartitionRole::Extended) || partition.fileSystem().type() == FileSystem::Unformatted) ? NULL : getPedFileSystemType(partition.fileSystem().type());

	PedPartition* pedPartition = ped_partition_new(pedDisk(), pedType, pedFsType, partition.firstSector(), partition.lastSector());

	if (pedPartition == NULL)
	{
		report.line() << i18nc("@info/plain", "Failed to create new partition <filename>%1</filename>.", partition.deviceNode());
		return false;
	}

	PedConstraint* pedConstraint = NULL;
	PedGeometry* pedGeometry = ped_geometry_new(pedDevice(), partition.firstSector(), partition.length());

	if (pedGeometry)
		pedConstraint = ped_constraint_exact(pedGeometry);

	if (pedConstraint == NULL)
	{
		report.line() << i18nc("@info/plain", "Failed to create a new partition: could not get geometry for constraint.");
		return -1;
	}

	if (ped_disk_add_partition(pedDisk(), pedPartition, pedConstraint))
		rval = pedPartition->num;
	else
		report.line() << i18nc("@info/plain", "Failed to add partition <filename>%1</filename> to device <filename>%2</filename>.", partition.deviceNode(), pedDisk()->dev->path);

	ped_constraint_destroy(pedConstraint);

	return rval;
}

bool LibPartedPartitionTable::deletePartition(Report& report, const Partition& partition)
{
	Q_ASSERT(partition.devicePath() == pedDevice()->path);

	bool rval = false;

	PedPartition* pedPartition = partition.roles().has(PartitionRole::Extended)
		? ped_disk_extended_partition(pedDisk())
		: ped_disk_get_partition_by_sector(pedDisk(), partition.firstSector());

	if (pedPartition)
	{
		rval = ped_disk_delete_partition(pedDisk(), pedPartition);

		if (!rval)
			report.line() << i18nc("@info/plain", "Could not delete partition <filename>%1</filename>.", partition.deviceNode());
	}
	else
		report.line() << i18nc("@info/plain", "Deleting partition failed: Partition to delete (<filename>%1</filename>) not found on disk.", partition.deviceNode());

	return rval;
}

bool LibPartedPartitionTable::updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end)
{
	Q_ASSERT(partition.devicePath() == pedDevice()->path);

	bool rval = false;

	PedPartition* pedPartition = (partition.roles().has(PartitionRole::Extended))
		? ped_disk_extended_partition(pedDisk())
		: ped_disk_get_partition_by_sector(pedDisk(), partition.firstSector());

	if (pedPartition)
	{
		if (PedGeometry* pedGeometry = ped_geometry_new(pedDevice(), sector_start, sector_end - sector_start + 1))
		{
			if (PedConstraint* pedConstraint = ped_constraint_exact(pedGeometry))
			{
				if (ped_disk_set_partition_geom(pedDisk(), pedPartition, pedConstraint, sector_start, sector_end))
					rval = true;
				else
					report.line() << i18nc("@info/plain", "Could not set geometry for partition <filename>%1</filename> while trying to resize/move it.", partition.deviceNode());
			}
			else
				report.line() << i18nc("@info/plain", "Could not get constraint for partition <filename>%1</filename> while trying to resize/move it.", partition.deviceNode());
		}
		else
			report.line() << i18nc("@info/plain", "Could not get geometry for partition <filename>%1</filename> while trying to resize/move it.", partition.deviceNode());
	}
	else
		report.line() << i18nc("@info/plain", "Could not open partition <filename>%1</filename> while trying to resize/move it.", partition.deviceNode());

	return rval;
}

bool LibPartedPartitionTable::clobberFileSystem(Report& report, const Partition& partition)
{
	bool rval = false;

	if (PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk(), partition.firstSector()))
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
						report.line() << i18nc("@info/plain", "Failed to erase reiser4 signature on partition <filename>%1</filename>.", partition.deviceNode());

					ped_device_close(pedDevice());
				}
			}
			else
				rval = true;
		}
		else
			report.line() << i18nc("@info/plain", "Failed to clobber file system on partition <filename>%1</filename>.", partition.deviceNode());
	}
	else
		report.line() << i18nc("@info/plain", "Could not delete file system on partition <filename>%1</filename>: Failed to get partition.", partition.deviceNode());

	return rval;
}

static void pedTimerHandler(PedTimer* pedTimer, void*)
{
	CoreBackendManager::self()->backend()->emitProgress(pedTimer->frac * 100);
}

bool LibPartedPartitionTable::resizeFileSystem(Report& report, const Partition& partition, qint64 newLength)
{
	bool rval = false;

	if (PedGeometry* originalGeometry = ped_geometry_new(pedDevice(), partition.fileSystem().firstSector(), partition.fileSystem().length()))
	{
		if (PedFileSystem* pedFileSystem = ped_file_system_open(originalGeometry))
		{
			if (PedGeometry* resizedGeometry = ped_geometry_new(pedDevice(), partition.fileSystem().firstSector(), newLength))
			{
 				PedTimer* pedTimer = ped_timer_new(pedTimerHandler, NULL);
				rval = ped_file_system_resize(pedFileSystem, resizedGeometry, pedTimer);
 				ped_timer_destroy(pedTimer);

				if (!rval)
					report.line() << i18nc("@info/plain", "Could not resize file system on partition <filename>%1</filename>.", partition.deviceNode());
			}
			else
				report.line() << i18nc("@info/plain", "Could not get geometry for resized partition <filename>%1</filename> while trying to resize the file system.", partition.deviceNode());

			ped_file_system_close(pedFileSystem);
		}
		else
			report.line() << i18nc("@info/plain", "Could not open partition <filename>%1</filename> while trying to resize the file system.", partition.deviceNode());
	}
	else
		report.line() << i18nc("@info/plain", "Could not read geometry for partition <filename>%1</filename> while trying to resize the file system.", partition.deviceNode());

	return rval;
}

FileSystem::Type LibPartedPartitionTable::detectFileSystemBySector(Report& report, const Device& device, qint64 sector)
{
	PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk(), sector);

	FileSystem::Type rval = FileSystem::Unknown;

	if (pedPartition)
		rval = LibPartedBackend::detectFileSystem(pedPartition);
	else
		report.line() << i18nc("@info/plain", "Could not determine file system of partition at sector %1 on device <filename>%2</filename>.", sector, device.deviceNode());

	return rval;
}
