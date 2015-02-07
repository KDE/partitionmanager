/*************************************************************************
 *  Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(LIBPARTED__H)

#define LIBPARTED__H

#include "backend/corebackend.h"

#include "core/partitiontable.h"
#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <parted/parted.h>

#include <QList>
#include <QVariant>
#include <qglobal.h>

class LibPartedDevice;
class LibPartedPartitionTable;
class LibPartedPartition;
class OperationStack;

class Device;
class KPluginFactory;
class QString;

/** Backend plugin for libparted.

	@author Volker Lanz <vl@fidra.de>
*/
class LibPartedBackend : public CoreBackend
{
	friend class KPluginFactory;
	friend class LibPartedPartition;
	friend class LibPartedDevice;
	friend class LibPartedPartitionTable;

	Q_DISABLE_COPY(LibPartedBackend)

	private:
		LibPartedBackend(QObject* parent, const QList<QVariant>& args);

	public:
		virtual void initFSSupport();

		virtual CoreBackendDevice* openDevice(const QString& device_node);
		virtual CoreBackendDevice* openDeviceExclusive(const QString& device_node);
		virtual bool closeDevice(CoreBackendDevice* core_device);
		virtual Device* scanDevice(const QString& device_node);
		virtual QList<Device*> scanDevices();

		static QString lastPartedExceptionMessage();

	private:
		static FileSystem::Type detectFileSystem(PedPartition* pedPartition);
		static PedPartitionFlag getPedFlag(PartitionTable::Flag flag);
		static void scanDevicePartitions(PedDevice* pedDevice, Device& d, PedDisk* pedDisk);
};

#endif
