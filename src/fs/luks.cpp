/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#include "fs/luks.h"

#include "util/capacity.h"
#include "util/externalcommand.h"

#include <QString>

namespace FS
{
	FileSystem::CommandSupportType luks::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Move = FileSystem::cmdSupportCore;
	FileSystem::CommandSupportType luks::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Copy = FileSystem::cmdSupportCore;
	FileSystem::CommandSupportType luks::m_Backup = FileSystem::cmdSupportCore;
	FileSystem::CommandSupportType luks::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_GetUUID = FileSystem::cmdSupportNone;

	luks::luks(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Luks)
	{
	}

	void luks::init()
	{
	}

	FileSystem::SupportTool luks::supportToolName() const
	{
		return SupportTool("cryptsetup", KUrl("https://code.google.com/p/cryptsetup/"));
	}

	qint64 luks::maxCapacity() const
	{
		 return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	QString luks::getCipherName(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxCipherName("(?:Cipher name:\\s+)([A-Za-z0-9-]+)");
			if (rxCipherName.indexIn(cmd.output()) > -1)
				return rxCipherName.cap(1);
		}
		return "---";
	}

	QString luks::getCipherMode(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxCipherMode("(?:Cipher mode:\\s+)([A-Za-z0-9-]+)");
			if (rxCipherMode.indexIn(cmd.output()) > -1)
				return rxCipherMode.cap(1);
		}
		return "---";
	}

	QString luks::getHashName(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxHash("(?:Hash spec:\\s+)([A-Za-z0-9-]+)");
			if (rxHash.indexIn(cmd.output()) > -1)
				return rxHash.cap(1);
		}
		return "---";
	}

	QString luks::getKeySize(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxKeySize("(?:MK bits:\\s+)(\\d+)");
			if (rxKeySize.indexIn(cmd.output()) > -1)
				return rxKeySize.cap(1);
		}
		return "---";
	}

	QString luks::getPayloadOffset(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxPayloadOffset("(?:Payload offset:\\s+)(\\d+)");
			if (rxPayloadOffset.indexIn(cmd.output()) > -1)
				return rxPayloadOffset.cap(1);
		}
		return "---";
	}
}
