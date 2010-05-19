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

#include "core/device.h"

#include "core/partitiontable.h"
#include "core/smartstatus.h"

#include "util/capacity.h"

#include <kdebug.h>
#include <klocale.h>

/** Constructs a Device with an empty PartitionTable.
	@param name the Device's name, usually some string defined by the manufacturer
	@param devicenode the Device's node, for example "/dev/sda"
	@param heads the number of heads in CHS notation
	@param numSectors the number of sectors in CHS notation
	@param cylinders the number of cylinders in CHS notation
	@param sectorSize the size of a sector in bytes
*/
Device::Device(const QString& name, const QString& devicenode, qint32 heads, qint32 numSectors, qint32 cylinders, qint64 sectorSize, const QString& iconname) :
	QObject(),
	m_Name(name),
	m_DeviceNode(devicenode),
	m_PartitionTable(NULL),
	m_Heads(heads),
	m_SectorsPerTrack(numSectors),
	m_Cylinders(cylinders),
	m_SectorSize(sectorSize),
	m_IconName(iconname.isEmpty() ? "drive-hardisk" : iconname),
	m_SmartStatus(new SmartStatus(devicenode))
{
}

/** Destructs a Device. */
Device::~Device()
{
	delete m_PartitionTable;
}

QString Device::prettyName() const
{
	return QString("%2 (%1, %3)").arg(name()).arg(deviceNode()).arg(Capacity(*this).toString());
}
