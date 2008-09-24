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

#include "fs/jfs.h"

#include "util/externalcommand.h"
#include "util/report.h"
#include "util/capacity.h"

#include <QStringList>
#include <QRegExp>

#include <klocale.h>
#include <ktempdir.h>

#include <unistd.h>

namespace FS
{
	FileSystem::SupportType jfs::m_GetUsed = FileSystem::SupportNone;
	FileSystem::SupportType jfs::m_GetLabel = FileSystem::SupportNone;
	FileSystem::SupportType jfs::m_Create = FileSystem::SupportNone;
	FileSystem::SupportType jfs::m_Grow = FileSystem::SupportNone;
	FileSystem::SupportType jfs::m_Move = FileSystem::SupportNone;
	FileSystem::SupportType jfs::m_Check = FileSystem::SupportNone;
	FileSystem::SupportType jfs::m_Copy = FileSystem::SupportNone;
	FileSystem::SupportType jfs::m_Backup = FileSystem::SupportNone;
	FileSystem::SupportType jfs::m_SetLabel = FileSystem::SupportNone;

	jfs::jfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Jfs)
	{
	}

	void jfs::init()
	{
		m_GetUsed = findExternal("jfs_debugfs") ? SupportExternal : SupportNone;
		m_SetLabel = m_GetLabel = findExternal("jfs_tune", QStringList() << "-V") ? SupportExternal : SupportNone;
		m_Create = findExternal("mkfs.jfs", QStringList() << "-V") ? SupportExternal : SupportNone;
		m_Grow = m_Check = findExternal("fsck.jfs", QStringList() << "-V") ? SupportExternal : SupportNone;
		m_Copy = m_Move = (m_Check != SupportNone) ? SupportInternal : SupportNone;
		m_Backup = SupportInternal;
	}

	qint64 jfs::minCapacity() const
	{
		return 16 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}
	
	qint64 jfs::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd("jfs_debugfs", QStringList() << deviceNode);

		if (cmd.start() && cmd.write("dm") == 2 && cmd.waitFor())
		{
			qint64 blockSize = -1;
			QRegExp rxBlockSize("Block Size: (\\d+)");

			if (rxBlockSize.indexIn(cmd.output()) != -1)
				blockSize = rxBlockSize.cap(1).toLongLong();

			qint64 nBlocks = -1;
			QRegExp rxnBlocks("dn_mapsize:\\s+0x([0-9a-f]+)");

			bool ok = false;
			if (rxnBlocks.indexIn(cmd.output()) != -1)
			{
				nBlocks = rxnBlocks.cap(1).toLongLong(&ok, 16);
				if (!ok)
					nBlocks = -1;
			}
			
			qint64 nFree = -1;
			QRegExp rxnFree("dn_nfree:\\s+0x([0-9a-f]+)");

			if (rxnFree.indexIn(cmd.output()) != -1)
			{
				nFree = rxnFree.cap(1).toLongLong(&ok, 16);
				if (!ok)
					nFree = -1;
			}

			if (nBlocks > -1 && blockSize > -1 && nFree > -1)
				return (nBlocks - nFree) * blockSize;
		}

		return -1;
	}

	QString jfs::readLabel(const QString& deviceNode) const
	{
		ExternalCommand cmd("jfs_tune", QStringList() << "-l" << deviceNode);

		if (cmd.run())
		{
			QRegExp rxLabel("Volume label:\\s+'(\\w+)'");

			if (rxLabel.indexIn(cmd.output()) != -1)
				return rxLabel.cap(1).simplified();
		}

		return QString();
	}

	bool jfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		return ExternalCommand(report, "jfs_tune", QStringList() << "-L" << newLabel << deviceNode).run(-1);
	}

	bool jfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, "fsck.jfs", QStringList() << "-f" << deviceNode);
		return cmd.run(-1) && (cmd.exitCode() == 0 || cmd.exitCode() == 1);
	}

	bool jfs::create(Report& report, const QString& deviceNode) const
	{
		return ExternalCommand(report, "mkfs.jfs", QStringList() << "-q" << deviceNode).run(-1);
	}
	
	bool jfs::resize(Report& report, const QString& deviceNode, qint64) const
	{
		KTempDir tempDir;
		if (!tempDir.exists())
		{
			report.line() << i18nc("@info/plain", "Resizing JFS file system on partition <filename>%1</filename> failed: Could not create temp dir.", deviceNode);
			return false;
		}

		bool rval = false;

		ExternalCommand mountCmd(report, "mount", QStringList() << "-v" << "-t" << "jfs" << deviceNode << tempDir.name());
		
		if (mountCmd.run(-1))
		{
			ExternalCommand resizeMountCmd(report, "mount", QStringList() << "-v" << "-t" << "jfs" << "-o" << "remount,resize" << deviceNode << tempDir.name());
		
			if (resizeMountCmd.run(-1))
				rval = true;
			else
				report.line() << i18nc("@info/plain", "Resizing JFS file system on partition <filename>%1</filename> failed: Remount failed.", deviceNode);
				
			ExternalCommand unmountCmd(report, "umount", QStringList() << tempDir.name());

			if (!unmountCmd.run(-1))
				report.line() << i18nc("@info/plain", "Warning: Resizing JFS file system on partition <filename>%1</filename>: Unmount failed.", deviceNode);
		}
		else
			report.line() << i18nc("@info/plain", "Resizing JFS file system on partition <filename>%1</filename> failed: Initial mount failed.", deviceNode);
		
		return rval;
	}
}
