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

#include "fs/xfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <QString>
#include <QStringList>
#include <QRegExp>

#include <ktempdir.h>
#include <klocale.h>

#include <unistd.h>

namespace FS
{
	FileSystem::CommandSupportType xfs::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_SetLabel = FileSystem::cmdSupportNone;

	xfs::xfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Xfs)
	{
	}

	void xfs::init()
	{
		m_GetLabel = cmdSupportCore;
		m_SetLabel = m_GetUsed = findExternal("xfs_db") ? cmdSupportFileSystem : cmdSupportNone;
		m_Create = findExternal("mkfs.xfs") ? cmdSupportFileSystem : cmdSupportNone;

		m_Check = findExternal("xfs_repair") ? cmdSupportFileSystem : cmdSupportNone;
		m_Grow = (findExternal("xfs_growfs", QStringList() << "-V") && m_Check != cmdSupportNone) ? cmdSupportFileSystem : cmdSupportNone;
		m_Copy = findExternal("xfs_copy") ? cmdSupportFileSystem : cmdSupportNone;
		m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Backup = cmdSupportCore;
	}

	qint64 xfs::minCapacity() const
	{
		return 32 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 xfs::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd("xfs_db", QStringList() << "-c" << "sb 0" << "-c" << "print" << deviceNode);

		if (cmd.run())
		{
			qint64 dBlocks = -1;
			QRegExp rxDBlocks("dblocks = (\\d+)");

			if (rxDBlocks.indexIn(cmd.output()) != -1)
				dBlocks = rxDBlocks.cap(1).toLongLong();

			qint64 blockSize = -1;
			QRegExp rxBlockSize("blocksize = (\\d+)");

			if (rxBlockSize.indexIn(cmd.output()) != -1)
				blockSize = rxBlockSize.cap(1).toLongLong();

			qint64 fdBlocks = -1;
			QRegExp rxFdBlocks("fdblocks = (\\d+)");

			if (rxFdBlocks.indexIn(cmd.output()) != -1)
				fdBlocks = rxFdBlocks.cap(1).toLongLong();

			if (dBlocks > -1 && blockSize > -1 && fdBlocks > -1)
				return (dBlocks - fdBlocks) * blockSize;
		}

		return -1;
	}

	bool xfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		return ExternalCommand(report, "xfs_db", QStringList() << "-x" << "-c" << "sb 0" << "-c" << QString("label " + newLabel) << deviceNode).run(-1);
	}

	bool xfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, "xfs_repair", QStringList() << "-v" << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool xfs::create(Report& report, const QString& deviceNode) const
	{
		return ExternalCommand(report, "mkfs.xfs", QStringList() << "-f" << deviceNode).run(-1);
	}

	bool xfs::copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const
	{
		ExternalCommand cmd(report, "xfs_copy", QStringList() << sourceDeviceNode << targetDeviceNode);

		// xfs_copy behaves a little strangely. It apparently kills itself at the end of main, causing QProcess
		// to report that it crashed.
		// See http://oss.sgi.com/archives/xfs/2004-11/msg00169.html
		// So we cannot rely on QProcess::exitStatus() and thus not on ExternalCommand::run() returning true.

		cmd.run(-1);
		return cmd.exitCode() == 0;
	}

	bool xfs::resize(Report& report, const QString& deviceNode, qint64) const
	{
		KTempDir tempDir;
		if (!tempDir.exists())
		{
			report.line() << i18nc("@info/plain", "Resizing XFS file system on partition <filename>%1</filename> failed: Could not create temp dir.", deviceNode);
			return false;
		}

		bool rval = false;

		ExternalCommand mountCmd(report, "mount", QStringList() << "-v" << "-t" << "xfs" << deviceNode << tempDir.name());

		if (mountCmd.run(-1))
		{
			ExternalCommand resizeCmd(report, "xfs_growfs", QStringList() << tempDir.name());

			if (resizeCmd.run(-1))
				rval = true;
			else
				report.line() << i18nc("@info/plain", "Resizing XFS file system on partition <filename>%1</filename> failed: xfs_growfs failed.", deviceNode);

			ExternalCommand unmountCmd(report, "umount", QStringList() << tempDir.name());

			if (!unmountCmd.run(-1))
				report.line() << i18nc("@info/plain", "Warning: Resizing XFS file system on partition <filename>%1</filename>: Unmount failed.", deviceNode);
		}
		else
			report.line() << i18nc("@info/plain", "Resizing XFS file system on partition <filename>%1</filename> failed: Initial mount failed.", deviceNode);

		return rval;
	}
}
