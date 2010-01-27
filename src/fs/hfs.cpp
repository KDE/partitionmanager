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

#include "fs/hfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>
#include <QRegExp>

namespace FS
{
	FileSystem::CommandSupportType hfs::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfs::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfs::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfs::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfs::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfs::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfs::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfs::m_Backup = FileSystem::cmdSupportNone;

	hfs::hfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Hfs)
	{
	}

	void hfs::init()
	{
		m_GetLabel = cmdSupportCore;
		m_Create = findExternal("hformat") ? cmdSupportFileSystem : cmdSupportNone;
		m_Check = findExternal("hfsck") ? cmdSupportFileSystem : cmdSupportNone;

		m_GetUsed = cmdSupportCore;
		m_Shrink = cmdSupportCore;

		m_Move = m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Backup = cmdSupportCore;
	}

	qint64 hfs::maxCapacity() const
	{
		 return 2 * Capacity::unitFactor(Capacity::Byte, Capacity::TiB);
	}

	bool hfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, "hfsck", QStringList() << "-v" << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool hfs::create(Report& report, const QString& deviceNode) const
	{
		return ExternalCommand(report, "hformat", QStringList() << deviceNode).run(-1);
	}
}
