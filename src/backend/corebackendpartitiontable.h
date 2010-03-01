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

#if !defined(COREBACKENDPARTITIONTABLE__H)

#define COREBACKENDPARTITIONTABLE__H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <qglobal.h>

class CoreBackendPartition;
class Report;
class Partition;

class LIBPARTITIONMANAGERPRIVATE_EXPORT CoreBackendPartitionTable
{
	public:
		virtual ~CoreBackendPartitionTable() {}

	public:
		virtual bool open() = 0;
		virtual bool commit(quint32 timeout = 10) = 0;

		virtual CoreBackendPartition* getExtendedPartition() = 0;
		virtual CoreBackendPartition* getPartitionBySector(qint64 sector) = 0;

		virtual bool deletePartition(Report& report, const Partition& partition) = 0;
		virtual bool clobberFileSystem(Report& report, const Partition& partition) = 0;
		virtual bool resizeFileSystem(Report& report, const Partition& partition, qint64 newLength) = 0;
		virtual FileSystem::Type detectFileSystemBySector(Report& report, const Device& device, qint64 sector) = 0;
		virtual bool createPartition(Report& report, const Partition& partition, quint32& new_number) = 0;

		virtual bool updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end) = 0;
};

#endif
