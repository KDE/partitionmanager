/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de                        *
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

#if !defined(COREBACKENDDEVICE__H)

#define COREBACKENDDEVICE__H

#include "util/libpartitionmanagerexport.h"

#include <QString>
#include <qglobal.h>

class CoreBackendPartition;
class CoreBackendPartitionTable;
class Partition;
class PartitionTable;
class Report;

class LIBPARTITIONMANAGERPRIVATE_EXPORT CoreBackendDevice
{
	public:
		CoreBackendDevice(const QString& device_node);
		virtual ~CoreBackendDevice() {}

	public:
		virtual const QString& deviceNode() const { return m_DeviceNode; }
		virtual bool isExclusive() const { return m_Exclusive; }

		virtual bool open() = 0;
		virtual bool openExclusive() = 0;
		virtual bool close() = 0;

		virtual CoreBackendPartitionTable* openPartitionTable() = 0;

		virtual bool createPartitionTable(Report& report, const PartitionTable& ptable) = 0;

		virtual bool readSectors(void* buffer, qint64 offset, qint64 numSectors) = 0;
		virtual bool writeSectors(void* buffer, qint64 offset, qint64 numSectors) = 0;

	protected:
		void setExclusive(bool b) { m_Exclusive = b; }

	private:
		const QString m_DeviceNode;
		bool m_Exclusive;
};

#endif
