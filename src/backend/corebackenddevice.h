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

#include <QString>
#include <qglobal.h>

class CoreBackendPartition;
class Partition;
class Report;

class CoreBackendDevice
{
	public:
		CoreBackendDevice(const QString& device_node);
		virtual ~CoreBackendDevice() {}

	public:
		virtual const QString& deviceNode() const { return m_DeviceNode; }

		virtual bool open() = 0;
		virtual bool close() = 0;
		virtual bool commit(quint32 timeout = 10) = 0;
		virtual CoreBackendPartition* getExtendedPartition() = 0;
		virtual CoreBackendPartition* getPartitionBySector(qint64 sector) = 0;

		virtual bool createPartition(Report& report, Partition& partition) = 0;

	private:
		const QString m_DeviceNode;
};


#endif
