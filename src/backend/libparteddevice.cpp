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

#include "backend/libparteddevice.h"
#include "backend/libpartedpartition.h"

#include "core/partition.h"

#include "fs/filesystem.h"

#include "util/externalcommand.h"
#include "util/globallog.h"
#include "util/report.h"

#include <klocale.h>
#include <kdebug.h>

#include <unistd.h>

LibPartedDevice::LibPartedDevice(const QString& device_node) :
	CoreBackendDevice(device_node)
{
}

LibPartedDevice::~LibPartedDevice()
{
	close();
}

bool LibPartedDevice::open()
{
	m_PedDevice = ped_device_get(deviceNode().toAscii());
	m_PedDisk = m_PedDevice ? ped_disk_new(m_PedDevice) : NULL;

	return pedDevice() != NULL && pedDisk() != NULL;
}

bool LibPartedDevice::close()
{
	if (pedDisk())
		ped_disk_destroy(pedDisk());

	m_PedDisk = NULL;
	m_PedDevice = NULL;

	return true;
}

bool LibPartedDevice::commit(quint32 timeout)
{
	if (pedDisk() == NULL)
		return false;

	bool rval = ped_disk_commit_to_dev(pedDisk());

	// The GParted authors have found a bug in libparted that causes it to intermittently
	// not commit changes to the Linux kernel, probably a race. Until this is fixed in
	// libparted, the following patch should help alleviate the consequences by just re-trying
	// committing to the OS if it fails the first time after a short pause.
	// See: http://git.gnome.org/browse/gparted/commit/?id=bf86fd3f9ceb0096dfe87a8c9a38403c13b13f00
	if (rval)
	{
		rval = ped_disk_commit_to_os(pedDisk());

		if (!rval)
		{
			sleep(1);
			rval = ped_disk_commit_to_os(pedDisk());
		}
	}

	if (!ExternalCommand("udevadm", QStringList() << "settle" << "--timeout=" + QString::number(timeout)).run() &&
			!ExternalCommand("udevsettle", QStringList() << "--timeout=" + QString::number(timeout)).run())
		sleep(timeout);

	return rval;
}

CoreBackendPartition* LibPartedDevice::getExtendedPartition()
{
	PedPartition* pedPartition = ped_disk_extended_partition(pedDisk());

	if (pedPartition == NULL)
		return NULL;

	return new LibPartedPartition(pedPartition);
}

CoreBackendPartition* LibPartedDevice::getPartitionBySector(qint64 sector)
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

bool LibPartedDevice::createPartition(Report& report, Partition& partition)
{
	Q_ASSERT(pedDevice());
	Q_ASSERT(pedDevice()->path);

	Q_ASSERT(partition.devicePath() == pedDevice()->path);

	bool rval = false;

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
		return false;
	}

	if (ped_disk_add_partition(pedDisk(), pedPartition, pedConstraint))
	{
		partition.setNumber(pedPartition->num);
		partition.setState(Partition::StateNone);

		partition.setFirstSector(pedPartition->geom.start);
		partition.setLastSector(pedPartition->geom.end);

		rval = true;
	}
	else
		report.line() << i18nc("@info/plain", "Failed to add partition <filename>%1</filename> to device <filename>%2</filename>.", partition.deviceNode(), pedDisk()->dev->path);

	ped_constraint_destroy(pedConstraint);

	return rval;
}
