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
	FileSystem::SupportType hfs::m_GetUsed = FileSystem::SupportNone;
	FileSystem::SupportType hfs::m_GetLabel = FileSystem::SupportNone;
	FileSystem::SupportType hfs::m_Create = FileSystem::SupportNone;
	FileSystem::SupportType hfs::m_Shrink = FileSystem::SupportNone;
	FileSystem::SupportType hfs::m_Move = FileSystem::SupportNone;
	FileSystem::SupportType hfs::m_Check = FileSystem::SupportNone;
	FileSystem::SupportType hfs::m_Copy = FileSystem::SupportNone;
	FileSystem::SupportType hfs::m_Backup = FileSystem::SupportNone;

	hfs::hfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Hfs)
	{
	}

	void hfs::init()
	{
		m_Create = findExternal("hformat") ? SupportExternal : SupportNone;
		m_Check = m_GetLabel = findExternal("hfsck") ? SupportExternal : SupportNone;

		m_GetUsed = SupportLibParted;
		m_Shrink = SupportLibParted;

		m_Move = m_Copy = (m_Check != SupportNone) ? SupportInternal : SupportNone;
		m_Backup = SupportInternal;
	}

	qint64 hfs::maxCapacity() const
	{
		 return 2 * Capacity::unitFactor(Capacity::Byte, Capacity::TiB);
	}
	
	QString hfs::readLabel(const QString& deviceNode) const
	{
		ExternalCommand cmd("hfsck", QStringList() << "-v" << deviceNode);

		if (cmd.run())
		{
			QRegExp rxVolumeName("drVN\\s*= \"(\\w+)\"");

			if (rxVolumeName.indexIn(cmd.output()) != -1)
				return rxVolumeName.cap(1);
		}

		return QString();
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
