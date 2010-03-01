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

/** @file
*/

#include "backend/dummy/dummybackend.h"
#include "backend/dummy/dummydevice.h"

#include "core/device.h"
#include "core/partition.h"
#include "core/partitiontable.h"

#include "util/globallog.h"

#include <QString>
#include <QStringList>

#include <kdebug.h>
#include <klocale.h>
#include <kpluginfactory.h>

K_PLUGIN_FACTORY(DummyBackendFactory, registerPlugin<DummyBackend>(); )
K_EXPORT_PLUGIN(DummyBackendFactory("pluginpmdummy"))


DummyBackend::DummyBackend(QObject*, const QList<QVariant>&) :
	CoreBackend()
{
}

QString DummyBackend::about() const
{
	return QString("DummyBackend");
}

Device* DummyBackend::scanDevice(const QString& device_node)
{
	Device* d = new Device("Dummy Device", QString("/tmp" + device_node), 255, 0xffff, 63, 512);
	CoreBackend::setPartitionTableForDevice(*d, new PartitionTable(PartitionTable::msdos_sectorbased, 2048, d->totalSectors() - 2048));
	CoreBackend::setPartitionTableMaxPrimaries(*d->partitionTable(), 128);
	d->partitionTable()->updateUnallocated(*d);
	return d;
}

CoreBackendDevice* DummyBackend::openDevice(const QString& device_node)
{
	DummyDevice* device = new DummyDevice(device_node);

	if (device == NULL || !device->open())
	{
		delete device;
		device = NULL;
	}

	return device;
}

CoreBackendDevice* DummyBackend::openDeviceExclusive(const QString& device_node)
{
	DummyDevice* device = new DummyDevice(device_node);

	if (device == NULL || !device->openExclusive())
	{
		delete device;
		device = NULL;
	}

	return device;
}

bool DummyBackend::closeDevice(CoreBackendDevice* core_device)
{
	return core_device->close();
}
