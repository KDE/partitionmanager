/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
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

/** @file
*/

#include "core/libparted.h"
#include "core/device.h"
#include "core/partition.h"
#include "core/partitiontable.h"
#include "core/operationstack.h"

#include "jobs/setpartflagsjob.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/globallog.h"

#include <QString>
#include <QStringList>
#include <QList>
#include <QFile>
#include <QMap>
#include <QRegExp>

#include <kdebug.h>
#include <klocale.h>
#include <kmountpoint.h>
#include <kdiskfreespaceinfo.h>

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/block.h>
#include <solid/storagedrive.h>

#include <parted/parted.h>
#include <unistd.h>

/** Callback to handle exceptions from libparted
	@param e the libparted exception to handle
*/
static PedExceptionOption pedExceptionHandler(PedException* e)
{
	/** @todo These messages should end up in the Report, not in the GlobalLog. */
	Log(Log::error) << i18nc("@info/plain", "LibParted Exception: %1", QString::fromLocal8Bit(e->message));
	return PED_EXCEPTION_UNHANDLED;
}

// --------------------------------------------------------------------------

// The following structs and the typedef come from libparted's internal gpt sources.
// It's very unfortunate there is no public API to get at the first and last usable
// sector for GPT a partition table, so this is the only (libparted) way to get that
// information (another way would be to read the GPT header and parse the
// information ourselves; if the libparted devs begin chaning these internal
// structs for each point release and break our code, we'll have to do just that).

typedef struct {
        uint32_t time_low;
        uint16_t time_mid;
        uint16_t time_hi_and_version;
        uint8_t  clock_seq_hi_and_reserved;
        uint8_t  clock_seq_low;
        uint8_t  node[6];
} /* __attribute__ ((packed)) */ efi_guid_t;


struct __attribute__ ((packed)) _GPTDiskData {
    PedGeometry data_area;
    int     entry_count;
    efi_guid_t  uuid;
};

typedef struct _GPTDiskData GPTDiskData;

// --------------------------------------------------------------------------

/** Get the first sector a Partition may cover on a given Device with a PartitionTable of
    type t.
    @param d the Device in question
    @param t the PartitonTable's type name (e.g. "msdos" or "gpt")
    @return the first sector usable by a Partition
*/
quint64 LibParted::firstUsableSector(const Device& d, const QString& t)
{
	PedDevice* pedDevice = ped_device_get(d.deviceNode().toAscii());
	PedDisk* pedDisk = pedDevice ? ped_disk_new(pedDevice) : NULL;

	quint64 rval = pedDisk->dev->bios_geom.sectors + 1;

	if (pedDisk && t == "gpt")
	{
		GPTDiskData* gpt_disk_data = reinterpret_cast<GPTDiskData*>(pedDisk->disk_specific);
		PedGeometry* geom = reinterpret_cast<PedGeometry*>(&gpt_disk_data->data_area);
		rval = geom->start;
	}

	return rval;
}

/** Get the last sector a Partition may cover on a given Device with a PartitionTable of
    type t.
    @param d the Device in question
    @param t the PartitonTable's type name (e.g. "msdos" or "gpt")
    @return the last sector usable by a Partition
*/
quint64 LibParted::lastUsableSector(const Device& d, const QString& t)
{
	PedDevice* pedDevice = ped_device_get(d.deviceNode().toAscii());
	PedDisk* pedDisk = pedDevice ? ped_disk_new(pedDevice) : NULL;

	quint64 rval = pedDisk->dev->bios_geom.sectors * pedDisk->dev->bios_geom.heads * pedDisk->dev->bios_geom.cylinders - 1;

	if (pedDisk && t == "gpt")
	{
		GPTDiskData* gpt_disk_data = reinterpret_cast<GPTDiskData*>(pedDisk->disk_specific);
		PedGeometry* geom = reinterpret_cast<PedGeometry*>(&gpt_disk_data->data_area);
		rval = geom->end;
	}

	return rval;
}

/** Reads sectors used on a FileSystem using libparted functions.
	@param pedDisk pointer to pedDisk  where the Partition and its FileSystem are
	@param p the Partition the FileSystem is on
	@return the number of sectors used
*/
static qint64 readSectorsUsedLibParted(PedDisk* pedDisk, const Partition& p)
{
	Q_ASSERT(pedDisk);

	qint64 rval = -1;

	PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk, p.firstSector());

	if (pedPartition)
	{
		PedFileSystem* pedFileSystem = ped_file_system_open(&pedPartition->geom);

		if (pedFileSystem)
		{
			if (PedConstraint* pedConstraint = ped_file_system_get_resize_constraint(pedFileSystem))
			{
				rval = pedConstraint->min_size;
				ped_constraint_destroy(pedConstraint);
			}

			ped_file_system_close(pedFileSystem);
		}
	}

	return rval;
}

/** Reads the sectors used in a FileSystem and stores the result in the Partition's FileSystem object.
	@param pedDisk pointer to pedDisk  where the Partition and its FileSystem are
	@param p the Partition the FileSystem is on
	@param mountPoint mount point of the partition in question
*/
static void readSectorsUsed(PedDisk* pedDisk, Partition& p, const QString& mountPoint)
{
	Q_ASSERT(pedDisk);

	const KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mountPoint);

	if (freeSpaceInfo.isValid())
		p.fileSystem().setSectorsUsed(freeSpaceInfo.used() / p.sectorSize());
	else if (p.fileSystem().supportGetUsed() == FileSystem::cmdSupportFileSystem)
		p.fileSystem().setSectorsUsed(p.fileSystem().readUsedCapacity(p.deviceNode()) / p.sectorSize());
	else if (p.fileSystem().supportGetUsed() == FileSystem::cmdSupportCore)
		p.fileSystem().setSectorsUsed(readSectorsUsedLibParted(pedDisk, p));
}

/** Scans a Device for Partitions.

	This function will scan a Device for all Partitions on it, detect the FileSystem for each Partition,
	try to determine the FileSystem usage, read the FileSystem label and store it all in newly created
	objects that are in the end added to the Device's PartitionTable.

	@param pedDevice libparted pointer to the device
	@param d Device
	@param pedDisk libparted pointer to the disk label
	@param mountInfo map of mount points
*/
static void scanDevicePartitions(PedDevice* pedDevice, Device& d, PedDisk* pedDisk)
{
	Q_ASSERT(pedDevice);
	Q_ASSERT(pedDisk);
	Q_ASSERT(d.partitionTable());

	PedPartition* pedPartition = NULL;

	KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName);
	mountPoints.append(KMountPoint::possibleMountPoints(KMountPoint::NeedRealDeviceName));

	while ((pedPartition = ped_disk_next_partition(pedDisk, pedPartition)))
	{
		if (pedPartition->num < 1)
			continue;

		PartitionRole::Roles r = PartitionRole::None;
		FileSystem::Type type = Job::detectFileSystem(pedDevice, pedPartition);

		switch(pedPartition->type)
		{
			case PED_PARTITION_NORMAL:
				r = PartitionRole::Primary;
				break;

			case PED_PARTITION_EXTENDED:
				r = PartitionRole::Extended;
				type = FileSystem::Extended;
				break;

			case PED_PARTITION_LOGICAL:
				r = PartitionRole::Logical;
				break;

			default:
				continue;
		}

		// Find an extended partition this partition is in.
		PartitionNode* parent = d.partitionTable()->findPartitionBySector(pedPartition->geom.start, PartitionRole(PartitionRole::Extended));

		// None found, so it's a primary in the device's partition table.
		if (parent == NULL)
			parent = d.partitionTable();

		const QString node = pedDisk->dev->path + QString::number(pedPartition->num);
		FileSystem* fs = FileSystemFactory::create(type, pedPartition->geom.start, pedPartition->geom.end);

		const QString mountPoint = mountPoints.findByDevice(node) ? mountPoints.findByDevice(node)->mountPoint() : QString();

		Partition* part = new Partition(parent, d, PartitionRole(r), fs, pedPartition->geom.start, pedPartition->geom.end,
				pedPartition->num, SetPartFlagsJob::availableFlags(pedPartition),
				QStringList() << mountPoint, ped_partition_is_busy(pedPartition), SetPartFlagsJob::activeFlags(pedPartition));

		readSectorsUsed(pedDisk, *part, mountPoint);

		if (fs->supportGetLabel() != FileSystem::cmdSupportNone)
			fs->setLabel(fs->readLabel(part->deviceNode()));

		if (fs->supportGetUUID() != FileSystem::cmdSupportNone)
			fs->setUUID(fs->readUUID(part->deviceNode()));

		parent->append(part);

		PartitionTable::isSnapped(d, *part);
	}

	d.partitionTable()->updateUnallocated(d);

	ped_disk_destroy(pedDisk);
}

/** Constructs a LibParted object. */
LibParted::LibParted()
{
	ped_exception_set_handler(pedExceptionHandler);
}

/** Scans for all available Devices on this computer.

	This method tries to find all Devices on the computer and creates new Device instances for each of them. It then calls scanDevicePartitions() to find all Partitions and FileSystems on each Device and set those up.

	The method will clear the list of operations and devices currently in the OperationStack given.

	@param ostack the OperationStack where the devices will be created
*/
void LibParted::scanDevices(OperationStack& ostack)
{
	ostack.clearOperations();
	ostack.clearDevices();

	// LibParted's ped_device_probe_all()
	// 1) segfaults when it finds "illegal" entries in /dev/mapper
	// 2) takes several minutes to time out if the BIOS says there's a floppy drive present
	//    when in fact there is none.
	// For that reason we scan devices on our own using Solid now.
	const QList<Solid::Device> driveList = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive, QString());

	foreach(const Solid::Device& solidDevice, driveList)
	{
		const Solid::StorageDrive* solidDrive = solidDevice.as<Solid::StorageDrive>();

		if (solidDrive->driveType() != Solid::StorageDrive::HardDisk)
			continue;

		const Solid::Block* solidBlock = solidDevice.as<Solid::Block>();
		PedDevice* pedDevice = ped_device_get(solidBlock->device().toLocal8Bit());

		Log(Log::information) << i18nc("@info/plain", "Device found: %1", pedDevice->model);

		Device* d = new Device(pedDevice->model, pedDevice->path, pedDevice->bios_geom.heads, pedDevice->bios_geom.sectors, pedDevice->bios_geom.cylinders, pedDevice->sector_size, solidDevice.icon());

		PedDisk* pedDisk = ped_disk_new(pedDevice);

		if (pedDisk)
		{
			d->setPartitionTable(new PartitionTable(pedDisk->type->name, firstUsableSector(*d, pedDisk->type->name), lastUsableSector(*d, pedDisk->type->name)));
			d->partitionTable()->setMaxPrimaries(ped_disk_get_max_primary_partition_count(pedDisk));

			scanDevicePartitions(pedDevice, *d, pedDisk);
		}

		ostack.addDevice(d);
	}

	ostack.sortDevices();
}

