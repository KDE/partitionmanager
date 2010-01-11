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

#include <parted/parted.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <mntent.h>

/** Callback to handle exceptions from libparted
	@param e the libparted exception to handle
*/
static PedExceptionOption pedExceptionHandler(PedException* e)
{
	/** @todo These messages should end up in the Report, not in the GlobalLog. */
	log(log::error) << i18nc("@info/plain", "LibParted Exception: %1", QString::fromLocal8Bit(e->message));
	return PED_EXCEPTION_UNHANDLED;
}

/** Finds a device by UUID.
	@param s the UUID
	@return the device node the UUID links to
*/
static QString findUuidDevice(const QString& s)
{
	const QString filename = "/dev/disk/by-uuid/" + QString(s).remove("UUID=");
	return QFile::exists(filename) ? QFile::symLinkTarget(filename) : "";
}

/** Finds a device by LABEL.
	@param s the label
	@return the device node the label links to
 */
static QString findLabelDevice(const QString& s)
{
	const QString filename = "/dev/disk/by-label/" + QString(s).remove("LABEL=");
	return QFile::exists(filename) ? QFile::symLinkTarget(filename) : "";
}

/** Reads mountpoints from a file.
	@param filename the name of the file to read mount points from
	@param result reference to QMap where the result will be stored
*/
static void readMountpoints(const QString& filename, QMap<QString, QStringList>& result)
{
	FILE* fp = setmntent(filename.toLocal8Bit(), "r");

	if (fp == NULL)
		return;

	struct mntent* p = NULL;

	while ((p = getmntent(fp)) != NULL)
	{
		QString device = p->mnt_fsname;

		if (device.startsWith("UUID="))
			device = findUuidDevice(device);

		if (device.startsWith("LABEL="))
			device = findLabelDevice(device);

		if (!device.isEmpty())
		{
			QString mountPoint = p->mnt_dir;

			if (QFile::exists(mountPoint) && result[device].indexOf(mountPoint) == -1)
				result[device].append(mountPoint);
		}
	}

	endmntent(fp);
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
	@param mountInfo map of mount points for the partition in question
*/
static void readSectorsUsed(PedDisk* pedDisk, Partition& p, QMap<QString, QStringList>& mountInfo)
{
	Q_ASSERT(pedDisk);

	struct statvfs sfs;

	if (p.isMounted() && mountInfo[p.deviceNode()].size() > 0 && statvfs(mountInfo[p.deviceNode()][0].toLatin1(), &sfs) == 0)
		p.fileSystem().setSectorsUsed((sfs.f_blocks - sfs.f_bfree) * sfs.f_bsize / p.sectorSize());
	else if (p.fileSystem().supportGetUsed() == FileSystem::SupportExternal)
		p.fileSystem().setSectorsUsed(p.fileSystem().readUsedCapacity(p.deviceNode()) / p.sectorSize());
	else if (p.fileSystem().supportGetUsed() == FileSystem::SupportLibParted)
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
static void scanDevicePartitions(PedDevice* pedDevice, Device& d, PedDisk* pedDisk, QMap<QString, QStringList>& mountInfo)
{
	Q_ASSERT(pedDevice);
	Q_ASSERT(pedDisk);
	Q_ASSERT(d.partitionTable());

	PedPartition* pedPartition = NULL;

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

		Partition* part = new Partition(parent, d, PartitionRole(r), fs, pedPartition->geom.start, pedPartition->geom.end,
				pedPartition->num, SetPartFlagsJob::availableFlags(pedPartition),
				mountInfo[node], ped_partition_is_busy(pedPartition), SetPartFlagsJob::activeFlags(pedPartition));

		readSectorsUsed(pedDisk, *part, mountInfo);

		if (fs->supportGetLabel() != FileSystem::SupportNone)
			fs->setLabel(fs->readLabel(part->deviceNode()));

		if (fs->supportGetUUID() != FileSystem::SupportNone)
			fs->setUUID(fs->readUUID(part->deviceNode()));

		parent->append(part);

		PartitionTable::isSnapped(d, *part);
	}

	d.partitionTable()->updateUnallocated(d);

	ped_disk_destroy(pedDisk);
}

/** Destructs a LibParted object. */
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
	QMap<QString, QStringList> mountInfo;

	readMountpoints("/proc/mounts", mountInfo);
	readMountpoints("/etc/mtab", mountInfo);
	readMountpoints("/etc/fstab", mountInfo);

	ostack.clearOperations();
	ostack.clearDevices();

	// LibParted's ped_device_probe_all()
	// 1) segfaults when it finds "illegal" entries in /dev/mapper
	// 2) takes several minutes to time out if the BIOS says there's a floppy drive present
	//    when in fact there is none.
	// For that reason we scan devices on our own if possible, using what the kernel knows and
	// tells us about in /proc/partitions.
	QFile partitions("/proc/partitions");
	if (partitions.open(QIODevice::ReadOnly))
	{
		QRegExp rxLine("\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s([^0-9]+)\\s+");
		QByteArray line;

		while (!(line = partitions.readLine()).isEmpty())
		{
			if (rxLine.indexIn(line) != -1)
			{
				const QString device = "/dev/" + rxLine.cap(4);
				// kDebug() << "device:" << device;
				ped_device_get(device.toLocal8Bit());
			}
		}

		partitions.close();
	}
	else
	{
		log(log::information) << i18nc("@info/plain", "Probing for devices using LibParted. This may crash or take a very long time. Read the manual's FAQ section for details.");
		ped_device_probe_all();
	}

	PedDevice* pedDevice = ped_device_get_next(NULL);

	while (pedDevice)
	{
		log(log::information) << i18nc("@info/plain", "Device found: %1", pedDevice->model);

		Device* d = new Device(pedDevice->model, pedDevice->path, pedDevice->bios_geom.heads, pedDevice->bios_geom.sectors, pedDevice->bios_geom.cylinders, pedDevice->sector_size);

		PedDisk* pedDisk = ped_disk_new(pedDevice);

		if (pedDisk)
		{
			d->setPartitionTable(new PartitionTable());
			d->partitionTable()->setMaxPrimaries(ped_disk_get_max_primary_partition_count(pedDisk));
			d->partitionTable()->setTypeName(pedDisk->type->name);

			scanDevicePartitions(pedDevice, *d, pedDisk, mountInfo);
		}

		// add this device if either there is a valid partition table or it's not
		// read only (read only devices without partition table are CD/DVD readers, writers etc)
		if (pedDisk || !pedDevice->read_only)
			ostack.addDevice(d);

		pedDevice = ped_device_get_next(pedDevice);
	}

	ostack.sortDevices();
}
