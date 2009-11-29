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

#include "fs/fat16.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <kdebug.h>

#include <QString>
#include <QStringList>
#include <QRegExp>

#include <ctime>

namespace FS
{
	FileSystem::SupportType fat16::m_GetUsed = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_GetLabel = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_Create = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_Grow = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_Shrink = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_Move = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_Check = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_Copy = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_Backup = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_UpdateUUID = FileSystem::SupportNone;
	FileSystem::SupportType fat16::m_GetUUID = FileSystem::SupportNone;

	fat16::fat16(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type t) :
		FileSystem(firstsector, lastsector, sectorsused, label, t)
	{
	}

	void fat16::init()
	{
		// There is no support for setting labels for FAT16 and FAT32 right now.
		// The mtools package is able to do that, but requires mappings from Unix device nodes
		// to Windows-like drive letters -- something we cannot support. It would, however,
		// probably be possible to implement the file system label stuff ourselves here.

		m_Create = findExternal("mkfs.msdos") ? SupportExternal : SupportNone;
		m_GetUsed = m_Check = findExternal("fsck.msdos", QStringList(), 2) ? SupportExternal : SupportNone;
		m_GetLabel = findIdUtil() ? SupportExternal : SupportNone;
		m_Grow = SupportLibParted;
		m_Shrink = SupportLibParted;
		m_Move = SupportInternal;
		m_Copy = SupportInternal;
		m_Backup = SupportInternal;
		m_UpdateUUID = findExternal("dd") ? SupportExternal : SupportNone;
		m_GetUUID = findIdUtil() ? SupportExternal : SupportNone;
	}

	qint64 fat16::minCapacity() const
	{
		 return 16 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 fat16::maxCapacity() const
	{
		 return 4 * Capacity::unitFactor(Capacity::Byte, Capacity::GiB);
	}

	qint64 fat16::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd("fsck.msdos", QStringList() << "-v" << deviceNode);

		if (cmd.run())
		{
			qint64 usedClusters = -1;
			QRegExp rxClusters("files, (\\d+)/\\d+ ");

			if (rxClusters.indexIn(cmd.output()) != -1)
				usedClusters = rxClusters.cap(1).toLongLong();

			qint64 clusterSize = -1;

			QRegExp rxClusterSize("(\\d+) bytes per cluster");

			if (rxClusterSize.indexIn(cmd.output()) != -1)
				clusterSize = rxClusterSize.cap(1).toLongLong();

			if (usedClusters > -1 && clusterSize > -1)
				return usedClusters * clusterSize;
		}

		return -1;
	}

	bool fat16::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, "fsck.msdos", QStringList() << "-a" << "-w" << "-v" << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool fat16::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, "mkfs.msdos", QStringList() << "-F16" << "-v" << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool fat16::updateUUID(Report& report, const QString& deviceNode) const
	{
		qint32 t = time(NULL);

		char uuid[4];
		for (quint32 i = 0; i < sizeof(uuid); i++, t >>= 8)
			uuid[i] = t & 0xff;

		ExternalCommand cmd(report, "dd", QStringList() << "of=" + deviceNode << "bs=1" << "count=4" << "seek=39");

		if (!cmd.start())
			return false;

		if (cmd.write(uuid, sizeof(uuid)) != sizeof(uuid))
			return false;

		return cmd.waitFor(-1);
	}
}
