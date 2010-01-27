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

#include "fs/linuxswap.h"

#include "util/externalcommand.h"

#include <klocale.h>

namespace FS
{
	FileSystem::CommandSupportType linuxswap::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_GetUUID = FileSystem::cmdSupportNone;

	linuxswap::linuxswap(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::LinuxSwap)
	{
	}

	void linuxswap::init()
	{
		m_SetLabel = m_Shrink = m_Grow = m_Create = (findExternal("mkswap")) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetLabel = cmdSupportCore;
		m_Copy = cmdSupportCore;
		m_Move = cmdSupportCore;
		m_GetUUID = cmdSupportCore;
	}

	bool linuxswap::create(Report& report, const QString& deviceNode) const
	{
		return ExternalCommand(report, "mkswap", QStringList() << deviceNode).run(-1);
	}

	bool linuxswap::resize(Report& report, const QString& deviceNode, qint64) const
	{
		const QString label = readLabel(deviceNode);

		QStringList args;
		if (!label.isEmpty())
			args << "-L" << label;
		args << deviceNode;

		return ExternalCommand(report, "mkswap", args).run(-1);
	}

	bool linuxswap::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		return ExternalCommand(report, "mkswap", QStringList() << "-L" << newLabel << deviceNode).run(-1);
	}

	QString linuxswap::mountTitle() const
	{
		return i18nc("@title:menu", "Activate swap");
	}

	QString linuxswap::unmountTitle() const
	{
		return i18nc("@title:menu", "Deactivate swap");
	}

	bool linuxswap::mount(const QString& deviceNode)
	{
		return ExternalCommand("swapon", QStringList() << deviceNode).run(-1);
	}

	bool linuxswap::unmount(const QString& deviceNode)
	{
		return ExternalCommand("swapoff", QStringList() << deviceNode).run(-1);
	}
}
