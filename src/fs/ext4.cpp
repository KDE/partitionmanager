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

#include "fs/ext4.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

namespace FS
{
	ext4::ext4(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		ext2(firstsector, lastsector, sectorsused, label, FileSystem::Ext4)
	{
	}

	qint64 ext4::maxCapacity() const
	{
		return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	bool ext4::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.ext4"), QStringList() << QStringLiteral("-q") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}
}
