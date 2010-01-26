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

#include "fs/ntfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"
#include "util/globallog.h"

#include <klocale.h>
#include <kdebug.h>

#include <QString>
#include <QStringList>
#include <QFile>
#include <QUuid>

#include <ctime>
#include <algorithm>

namespace FS
{
	FileSystem::SupportType ntfs::m_GetUsed = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_GetLabel = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_Create = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_Grow = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_Shrink = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_Move = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_Check = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_Copy = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_Backup = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_SetLabel = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_UpdateUUID = FileSystem::SupportNone;
	FileSystem::SupportType ntfs::m_GetUUID = FileSystem::SupportNone;

	ntfs::ntfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Ntfs)
	{
	}

	void ntfs::init()
	{
		m_Shrink = m_Grow = m_Check = m_GetUsed = findExternal("ntfsresize") ? SupportExternal : SupportNone;
		m_GetLabel = SupportInternal;
		m_SetLabel = findExternal("ntfslabel") ? SupportExternal : SupportNone;
		m_Create = findExternal("mkfs.ntfs") ? SupportExternal : SupportNone;
		m_Copy = findExternal("ntfsclone") ? SupportExternal : SupportNone;
		m_Backup = SupportInternal;
		m_UpdateUUID = findExternal("dd") ? SupportExternal : SupportNone;
		m_Move = (m_Check != SupportNone) ? SupportInternal : SupportNone;
		m_GetUUID = SupportInternal;
	}

	qint64 ntfs::maxCapacity() const
	{
		return 256 * Capacity::unitFactor(Capacity::Byte, Capacity::TiB);
	}

	qint64 ntfs::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd("ntfsresize", QStringList() << "--info" << "--force" << "--no-progress-bar" << deviceNode);

		if (cmd.run())
		{
			qint64 usedBytes = -1;
			QRegExp rxUsedBytes("resize at (\\d+) bytes");

			if (rxUsedBytes.indexIn(cmd.output()) != -1)
				usedBytes = rxUsedBytes.cap(1).toLongLong();

			if (usedBytes > -1)
				return usedBytes;
		}

		return -1;
	}

	bool ntfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand writeCmd(report, "ntfslabel", QStringList() << "--force" << deviceNode << newLabel.simplified());
		writeCmd.setProcessChannelMode(QProcess::SeparateChannels);

		if (!writeCmd.run(-1))
			return false;

		ExternalCommand testCmd("ntfslabel", QStringList() << "--force" << deviceNode);
		testCmd.setProcessChannelMode(QProcess::SeparateChannels);

		if (!testCmd.run(-1))
			return false;

		return testCmd.output().simplified() == newLabel.simplified();
	}

	bool ntfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, "ntfsresize", QStringList() << "-P" << "-i" << "-f" << "-v" << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool ntfs::create(Report& report, const QString& deviceNode) const
	{
		return ExternalCommand(report, "mkfs.ntfs", QStringList() << "-f" << "-vv" << deviceNode).run(-1);
	}

	bool ntfs::copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const
	{
 		ExternalCommand cmd(report, "ntfsclone", QStringList() << "-f" << "--overwrite" << targetDeviceNode << sourceDeviceNode);

 		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool ntfs::resize(Report& report, const QString& deviceNode, qint64 length) const
	{
		QStringList args;
		args << "-P" << "-f" << deviceNode << "-s" << QString::number(length);

		QStringList dryRunArgs = args;
		dryRunArgs << "-n";
		ExternalCommand cmdDryRun("ntfsresize", dryRunArgs);

		if (cmdDryRun.run(-1) && cmdDryRun.exitCode() == 0)
		{
			ExternalCommand cmd(report, "ntfsresize", args);
			return cmd.run(-1) && cmd.exitCode() == 0;
		}

		return false;
	}

	bool ntfs::updateUUID(Report& report, const QString& deviceNode) const
	{
		QUuid uuid = QUuid::createUuid();

		ExternalCommand cmd(report, "dd", QStringList() << "of=" + deviceNode << "bs=1" << "count=8" << "seek=72");

		if (!cmd.start())
			return false;

		if (cmd.write(reinterpret_cast<char*>(&uuid.data4[0]), 8) != 8)
			return false;

		return cmd.waitFor(-1);
	}

	bool ntfs::updateBootSector(Report& report, const QString& deviceNode) const
	{
		report.line() << i18nc("@info/plain", "Updating boot sector for NTFS file system on partition <filename>%1</filename>.", deviceNode);

		quint32 n = firstSector();
		char* s = reinterpret_cast<char*>(&n);

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
		std::swap(s[0], s[3]);
		std::swap(s[1], s[2]);
#endif

		QFile device(deviceNode);
		if (!device.open(QFile::ReadWrite | QFile::Unbuffered))
		{
			Log() << i18nc("@info/plain", "Could not open partition <filename>%1</filename> for writing when trying to update the NTFS boot sector.", deviceNode);
			return false;
		}

		if (!device.seek(0x1c))
		{
			Log() << i18nc("@info/plain", "Could not seek to position 0x1c on partition <filename>%1</filename> when trying to update the NTFS boot sector.", deviceNode);
			return false;
		}

		if (device.write(s, 4) != 4)
		{
			Log() << i18nc("@info/plain", "Could not write new start sector to partition <filename>%1</filename> when trying to update the NTFS boot sector.", deviceNode);
			return false;
		}

		Log() << i18nc("@info/plain", "Updated NTFS boot sector for partition <filename>%1</filename> successfully.", deviceNode);

		return true;
	}
}
