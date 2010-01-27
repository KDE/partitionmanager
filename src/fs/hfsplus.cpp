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

#include "fs/hfsplus.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

namespace FS
{
	FileSystem::CommandSupportType hfsplus::m_GetUsed = FileSystem::SupportNone;
	FileSystem::CommandSupportType hfsplus::m_Shrink = FileSystem::SupportNone;
	FileSystem::CommandSupportType hfsplus::m_Move = FileSystem::SupportNone;
	FileSystem::CommandSupportType hfsplus::m_Check = FileSystem::SupportNone;
	FileSystem::CommandSupportType hfsplus::m_Copy = FileSystem::SupportNone;
	FileSystem::CommandSupportType hfsplus::m_Backup = FileSystem::SupportNone;

	hfsplus::hfsplus(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::HfsPlus)
	{
	}

	void hfsplus::init()
	{
		m_Check = findExternal("hpfsck") ? SupportExternal : SupportNone;

		m_GetUsed = SupportInternal;
		m_Shrink = SupportInternal;
		m_Copy = (m_Check != SupportNone) ? SupportInternal : SupportNone;
		m_Move = (m_Check != SupportNone) ? SupportInternal : SupportNone;
		m_Backup = SupportInternal;
	}

	qint64 hfsplus::maxCapacity() const
	{
		return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}
	
	bool hfsplus::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, "hpfsck", QStringList() << "-v" << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}
}
