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

#include "jobs/job.h"

#include "core/device.h"
#include "core/copysource.h"
#include "core/copytarget.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"

#include "util/externalcommand.h"
#include "util/report.h"

#include <QIcon>
#include <QTime>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include <unistd.h>
#include <blkid/blkid.h>

static const struct
{
	FileSystem::Type type;
	QString name;
} mapFileSystemTypeToLibPartedName[] =
{
	{ FileSystem::Ext2, "ext2" },
	{ FileSystem::Ext3, "ext3" },
	{ FileSystem::Ext3, "ext4" },
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

Job::Job() :
	m_PedDevice(NULL),
	m_PedDisk(NULL),
	m_Status(Pending)
{
}

Job::~Job()
{
	closePed();
}

bool Job::openPed(const QString& path, bool diskFailOk)
{
	m_PedDevice = ped_device_get(path.toAscii());
	m_PedDisk = m_PedDevice ? ped_disk_new(m_PedDevice) : NULL;

	return m_PedDevice != NULL && (diskFailOk || m_PedDisk != NULL);
}

void Job::closePed()
{
	if (m_PedDisk)
		ped_disk_destroy(m_PedDisk);

	m_PedDisk = NULL;
	m_PedDevice = NULL;
}

bool Job::commit(quint32 timeout)
{
	return commit(m_PedDisk, timeout);
}

bool Job::commit(PedDisk* disk, quint32 timeout)
{
	if (disk == NULL)
		return false;

	bool rval = ped_disk_commit_to_dev(disk);

	rval = ped_disk_commit_to_os(disk) && rval;

	if (!ExternalCommand("udevadm", QStringList() << "settle" << "--timeout=" + QString::number(timeout)).run() &&
			!ExternalCommand("udevsettle", QStringList() << "--timeout=" + QString::number(timeout)).run())
		sleep(timeout);

	return rval;
}

PedFileSystemType* Job::getPedFileSystemType(FileSystem::Type t)
{
	for (quint32 i = 0; i < sizeof(mapFileSystemTypeToLibPartedName) / sizeof(mapFileSystemTypeToLibPartedName[0]); i++)
		if (mapFileSystemTypeToLibPartedName[i].type == t)
			return ped_file_system_type_get(mapFileSystemTypeToLibPartedName[i].name.toAscii());

	// if we didn't find anything, go with ext2 as a safe fallback
	return ped_file_system_type_get("ext2");
}

bool Job::copyBlocks(Report& report, CopyTarget& target, CopySource& source)
{
	/** @todo copyBlocks() assumes that source.sectorSize() == target.sectorSize(). */

	if (source.sectorSize() != target.sectorSize())
	{
		report.line() << i18nc("@info/plain", "The sector size in the source and target for copying are not the same. This is currently unsupported.");
		return false;
	}

	bool rval = true;
	const qint64 blockSize = 16065 * 8; // number of sectors per block to copy
	const qint64 blocksToCopy = source.length() / blockSize;

	qint64 readOffset = source.firstSector();
	qint64 writeOffset = target.firstSector();
	qint32 copyDir = 1;

	if (target.firstSector() > source.firstSector())
	{
		readOffset = source.firstSector() + source.length() - blockSize;
		writeOffset = target.firstSector() + source.length() - blockSize;
		copyDir = -1;
	}

	report.line() << i18nc("@info/plain", "Copying %1 blocks (%2 sectors) from %3 to %4, direction: %5.", blocksToCopy, source.length(), readOffset, writeOffset, copyDir);

	qint64 blocksCopied = 0;

	void* buffer = malloc(blockSize * source.sectorSize());
	int percent = 0;
	QTime t;
	t.start();

	while (blocksCopied < blocksToCopy)
	{
		if (!(rval = source.readSectors(buffer, readOffset + blockSize * blocksCopied * copyDir, blockSize)))
			break;

		if (!(rval = target.writeSectors(buffer, writeOffset + blockSize * blocksCopied * copyDir, blockSize)))
			break;

		if (++blocksCopied * 100 / blocksToCopy != percent)
		{
			percent = blocksCopied * 100 / blocksToCopy;

			if (percent % 5 == 0 && t.elapsed() > 1000)
			{
				const qint64 mibsPerSec = (blocksCopied * blockSize * source.sectorSize() / 1024 / 1024) / (t.elapsed() / 1000);
				const qint64 estSecsLeft = (100 - percent) * t.elapsed() / percent / 1000;
				report.line() << i18nc("@info/plain", "Copying %1 MiB/second, estimated time left: %2", mibsPerSec, QTime(0, 0).addSecs(estSecsLeft).toString());
			}
 			emit progress(percent);
		}
	}

	const qint64 lastBlock = source.length() % blockSize;

	// copy the remainder
	if (rval && lastBlock > 0)
	{
		Q_ASSERT(lastBlock < blockSize);

		if (lastBlock >= blockSize)
			kWarning() << "lastBlock: " << lastBlock << ", blockSize: " << blockSize;

		const qint64 lastBlockReadOffset = copyDir > 0 ? readOffset + blockSize * blocksCopied : source.firstSector();
		const qint64 lastBlockWriteOffset = copyDir > 0 ? writeOffset + blockSize * blocksCopied : target.firstSector();

		report.line() << i18nc("@info/plain", "Copying remainder of block size %1 from  %2 to %3.", lastBlock, lastBlockReadOffset, lastBlockWriteOffset);

		rval = source.readSectors(buffer, lastBlockReadOffset, lastBlock);

		if (rval)
			rval = target.writeSectors(buffer, lastBlockWriteOffset, lastBlock);

		if (rval)
			emit progress(100);
	}

	free(buffer);

 	report.line() << i18ncp("@info/plain, argument 2 is a string such as 7 sectors (localized accordingly)", "Copying 1 block (%2) finished.", "Copying %1 blocks (%2) finished.", blocksCopied, i18np("1 sector", "%1 sectors", target.sectorsWritten()));

	return rval;
}

bool Job::rollbackCopyBlocks(Report& report, CopyTarget& origTarget, CopySource& origSource)
{
	if (!origSource.overlaps(origTarget))
	{
		report.line() << i18nc("@info/plain", "Source and target for copying do not overlap: Rollback is not required.");
		return true;
	}

	try
	{
		CopySourceDevice& csd = dynamic_cast<CopySourceDevice&>(origSource);
		CopyTargetDevice& ctd = dynamic_cast<CopyTargetDevice&>(origTarget);

		// default: use values as if we were copying from front to back.
		qint64 undoSourceFirstSector = origTarget.firstSector();
		qint64 undoSourceLastSector = origTarget.firstSector() + origTarget.sectorsWritten() - 1;

		qint64 undoTargetFirstSector = origSource.firstSector();
		qint64 undoTargetLastSector = origSource.firstSector() + origTarget.sectorsWritten() - 1;

		if (origTarget.firstSector() > origSource.firstSector())
		{
			// we were copying from back to front
			undoSourceFirstSector = origTarget.firstSector() + origSource.length() - origTarget.sectorsWritten();
			undoSourceLastSector = origTarget.firstSector() + origSource.length() - 1;

			undoTargetFirstSector = origSource.lastSector() - origTarget.sectorsWritten() + 1;
			undoTargetLastSector = origSource.lastSector();
		}

		report.line() << i18nc("@info/plain", "Rollback from: First sector: %1, last sector: %2.", undoSourceFirstSector, undoSourceLastSector);
		report.line() << i18nc("@info/plain", "Rollback to: First sector: %1, last sector: %2.", undoTargetFirstSector, undoTargetLastSector);

		CopySourceDevice undoSource(ctd.device(), undoSourceFirstSector, undoSourceLastSector);
		if (!undoSource.open())
		{
			report.line() << i18nc("@info/plain", "Could not open device <filename>%1</filename> to rollback copying.", ctd.device().deviceNode());
			return false;
		}

		CopyTargetDevice undoTarget(csd.device(), undoTargetFirstSector, undoTargetLastSector);
		if (!undoTarget.open())
		{
			report.line() << i18nc("@info/plain", "Could not open device <filename>%1</filename> to rollback copying.", csd.device().deviceNode());
			return false;
		}

		return copyBlocks(report, undoTarget, undoSource);
	}
	catch ( ... )
	{
		report.line() << i18nc("@info/plain", "Rollback failed: Source or target are not devices.");
	}

	return false;
}

FileSystem::Type Job::detectFileSystemBySector(Report& report, Device& device, qint64 sector)
{
	if (!openPed(device.deviceNode()))
		return FileSystem::Unknown;

	PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk(), sector);

	FileSystem::Type rval = FileSystem::Unknown;

	if (pedPartition)
		rval = detectFileSystem(pedDevice(), pedPartition);
	else
		report.line() << i18nc("@info/plain", "Could not determine file system of partition at sector %1 on device <filename>%2</filename>.", sector, device.deviceNode());

	closePed();

	return rval;
}

/** Detects the type of a FileSystem given a PedDevice and a PedPartition
	@param pedDevice pointer to the pedDevice. Must not be NULL.
	@param pedPartition pointer to the pedPartition. Must not be NULL
	@return the detected FileSystem type (FileSystem::Unknown if not detected)
*/
FileSystem::Type Job::detectFileSystem(PedDevice* pedDevice, PedPartition* pedPartition)
{
	FileSystem::Type rval = FileSystem::Unknown;
	const QString s = pedPartition->fs_type ? pedPartition->fs_type->name : QString();

	if (s == "extended") rval = FileSystem::Extended;
	else if (s == "ext2") rval = FileSystem::Ext2;
	else if (s == "ext3") rval = FileSystem::Ext3;
	else if (s == "linux-swap") rval = FileSystem::LinuxSwap;
	else if (s == "fat16") rval = FileSystem::Fat16;
	else if (s == "fat32") rval = FileSystem::Fat32;
	else if (s == "ntfs") rval = FileSystem::Ntfs;
	else if (s == "reiserfs") rval = FileSystem::ReiserFS;
	else if (s == "xfs") rval = FileSystem::Xfs;
	else if (s == "jfs") rval = FileSystem::Jfs;
	else if (s == "hfs") rval = FileSystem::Hfs;
	else if (s == "hfs+") rval = FileSystem::HfsPlus;
	else if (s == "ufs") rval = FileSystem::Ufs;

	if (rval == FileSystem::Unknown)
	{
		// detect Reiser4, libparted doesn't deal with it
		char* buf = static_cast<char*>(malloc(pedDevice->sector_size));

		if (buf != NULL)
		{
			ped_device_open(pedDevice);
			ped_geometry_read(&pedPartition->geom, buf, 128, 1);
			ped_device_close(pedDevice);

			if (QString(buf) == "ReIsEr4")
				rval = FileSystem::Reiser4;

			free(buf);
		}
	}

	if (rval == FileSystem::Ext3)
	{
		blkid_cache cache;
		char* pedPath = NULL;

		if (blkid_get_cache(&cache, NULL) == 0 && (pedPath = ped_partition_get_path(pedPartition)))
		{
			blkid_dev dev;

			if ((dev = blkid_get_dev(cache, pedPath, BLKID_DEV_NORMAL)) != NULL &&
					(blkid_dev_has_tag(dev, "TYPE", "ext4")
						|| blkid_dev_has_tag(dev, "TYPE", "ext4dev")
					)
			)
				rval = FileSystem::Ext4;

			blkid_put_cache(cache);

			free(pedPath);
		}
	}

	return rval;
}

void Job::emitProgress(int i)
{
	 emit progress(i);
}

void Job::pedTimerHandler(PedTimer* pedTimer, void* ctx)
{
	if (ctx)
	{
		Job* job = reinterpret_cast<Job*>(ctx);

		if (job)
			job->emitProgress(pedTimer->frac * 100);
	}
}

Report* Job::jobStarted(Report& parent)
{
	emit started();

 	return parent.newChild(i18nc("@info/plain", "Job: %1", description()));
}

void Job::jobFinished(Report& report, bool b)
{
	setStatus(b ? Success : Error);
	emit progress(numSteps());
	emit finished();

	report.setStatus(i18nc("@info/plain job status (error, warning, ...)", "%1: %2", description(), statusText()));
}

/** @return the Job's current status icon */
QIcon Job::statusIcon() const
{
	static const char* icons[] =
	{
		"dialog-information",
		"dialog-ok",
		"dialog-error"
	};

	Q_ASSERT(status() >= 0 && static_cast<quint32>(status()) < sizeof(icons) / sizeof(icons[0]));

	if (status() < 0 || static_cast<quint32>(status()) >= sizeof(icons) / sizeof(icons[0]))
		return QIcon();

	return SmallIcon(icons[status()]);
}

/** @return the Job's current status text */
QString Job::statusText() const
{
	static const QString s[] =
	{
		i18nc("@info:progress job", "Pending"),
		i18nc("@info:progress job", "Success"),
		i18nc("@info:progress job", "Error")
	};

	Q_ASSERT(status() >= 0 && static_cast<quint32>(status()) < sizeof(s) / sizeof(s[0]));

	if (status() < 0 || static_cast<quint32>(status()) >= sizeof(s) / sizeof(s[0]))
		return QString();

	return s[status()];
}
