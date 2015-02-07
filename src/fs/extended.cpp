/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#include "fs/extended.h"

namespace FS
{
	FileSystem::CommandSupportType extended::m_Create = FileSystem::cmdSupportFileSystem;
	FileSystem::CommandSupportType extended::m_Grow = FileSystem::cmdSupportCore;
	FileSystem::CommandSupportType extended::m_Shrink = FileSystem::cmdSupportCore;
	FileSystem::CommandSupportType extended::m_Move = FileSystem::cmdSupportCore;

	extended::extended(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Extended)
	{
	}
	
	bool extended::create(Report&, const QString&) const
	{
		return true;
	}
}

