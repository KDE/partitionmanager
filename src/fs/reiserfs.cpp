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

#include "fs/reiserfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QString>
#include <QStringList>
#include <QRegExp>

#include <uuid/uuid.h>

namespace FS
{
	FileSystem::SupportType reiserfs::m_GetUsed = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_GetLabel = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_Create = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_Grow = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_Shrink = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_Move = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_Check = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_Copy = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_Backup = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_SetLabel = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_UpdateUUID = FileSystem::SupportNone;
	FileSystem::SupportType reiserfs::m_GetUUID = FileSystem::SupportNone;

	reiserfs::reiserfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::ReiserFS)
	{
	}

	void reiserfs::init()
	{
		m_GetLabel = m_GetUsed = findExternal("debugreiserfs", QStringList(), 16) ? SupportExternal : SupportNone;
		m_SetLabel = findExternal("reiserfstune") ? SupportExternal : SupportNone;
		m_Create = findExternal("mkfs.reiserfs") ? SupportExternal : SupportNone;
		m_Check = findExternal("fsck.reiserfs") ? SupportExternal : SupportNone;
		m_Move = m_Copy = (m_Check != SupportNone) ? SupportInternal : SupportNone;
		m_Grow = findExternal("resize_reiserfs", QStringList(), 16) ? SupportExternal : SupportNone;
		m_Shrink = (m_GetUsed != SupportNone && m_Grow != SupportNone) ? SupportExternal : SupportNone;
		m_Backup = SupportInternal;
		m_UpdateUUID = findExternal("reiserfstune") ? SupportExternal : SupportNone;
		m_GetUUID = findExternal("vol_id") ? SupportExternal : SupportNone;
	}

	qint64 reiserfs::minCapacity() const
	{
		return 32 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 reiserfs::maxCapacity() const
	{
		return 16 * Capacity::unitFactor(Capacity::Byte, Capacity::TiB);
	}

	qint64 reiserfs::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd("debugreiserfs", QStringList() << deviceNode);

		if (cmd.run())
		{
			qint64 blockCount = -1;
			QRegExp rxBlockCount("Count of blocks[^:]+: (\\d+)");

			if (rxBlockCount.indexIn(cmd.output()) != -1)
				blockCount = rxBlockCount.cap(1).toLongLong();

			qint64 blockSize = -1;
			QRegExp rxBlockSize("Blocksize: (\\d+)");

			if (rxBlockSize.indexIn(cmd.output()) != -1)
				blockSize = rxBlockSize.cap(1).toLongLong();

			qint64 freeBlocks = -1;
			QRegExp rxFreeBlocks("Free blocks[^:]+: (\\d+)");

			if (rxFreeBlocks.indexIn(cmd.output()) != -1)
				freeBlocks = rxFreeBlocks.cap(1).toLongLong();

			if (blockCount > -1 && blockSize > -1 && freeBlocks > -1)
				return (blockCount - freeBlocks) * blockSize;
		}

		return -1;
	}

	QString reiserfs::readLabel(const QString& deviceNode) const
	{
		ExternalCommand cmd("debugreiserfs", QStringList() << deviceNode);

		if (cmd.run())
		{
			QRegExp rxLabel("LABEL: (\\w+)");

			if (rxLabel.indexIn(cmd.output()) != -1)
				return rxLabel.cap(1).simplified();
		}

		return QString();
	}

	bool reiserfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		return ExternalCommand(report, "reiserfstune", QStringList() << "-l" << newLabel << deviceNode).run(-1);
	}

	bool reiserfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, "fsck.reiserfs", QStringList() << "--fix-fixable" << "-q" << "-y" << deviceNode);
		return cmd.run(-1) && (cmd.exitCode() == 0 || cmd.exitCode() == 1 || cmd.exitCode() == 256);
	}

	bool reiserfs::create(Report& report, const QString& deviceNode) const
	{
		return ExternalCommand(report, "mkfs.reiserfs", QStringList() << "-f" << deviceNode).run(-1);
	}

	bool reiserfs::resize(Report& report, const QString& deviceNode, qint64 length) const
	{
		ExternalCommand cmd(report, "resize_reiserfs", QStringList() << deviceNode << "-q" << "-s" << QString::number(length));

		bool rval = cmd.start(-1);

		if (!rval)
			return false;

		if (cmd.write("y\n", 2) != 2)
			return false;

		return cmd.waitFor(-1) && (cmd.exitCode() == 0 || cmd.exitCode() == 256);
	}

	bool reiserfs::updateUUID(Report& report, const QString& deviceNode) const
	{
		unsigned char uuid[16];
		uuid_generate(uuid);
		char uuid_ascii[37];
		uuid_unparse(uuid, uuid_ascii);

		return ExternalCommand(report, "reiserfstune", QStringList() << "-u" << uuid_ascii << deviceNode).run(-1);
	}
}
