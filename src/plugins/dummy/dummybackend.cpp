/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

/** @file
*/

#include "plugins/dummy/dummybackend.h"
#include "plugins/dummy/dummydevice.h"

#include "core/device.h"
#include "core/partition.h"
#include "core/partitiontable.h"

#include "util/globallog.h"

#include <QString>
#include <QStringList>

#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(DummyBackendFactory, "pmdummybackendplugin.json", registerPlugin<DummyBackend>(); )


DummyBackend::DummyBackend(QObject*, const QList<QVariant>&) :
	CoreBackend()
{
}

void DummyBackend::initFSSupport()
{
}

QList<Device*> DummyBackend::scanDevices()
{
	QList<Device*> result;
	result.append(scanDevice(QStringLiteral("/dev/sda")));

	emitScanProgress(QStringLiteral("/dev/sda"), 100);

	return result;
}

Device* DummyBackend::scanDevice(const QString& device_node)
{
	Device* d = new Device(QStringLiteral("Dummy Device"), QStringLiteral("/tmp") + device_node, 255, 30, 63, 512);
	CoreBackend::setPartitionTableForDevice(*d, new PartitionTable(PartitionTable::msdos_sectorbased, 2048, d->totalSectors() - 2048));
	CoreBackend::setPartitionTableMaxPrimaries(*d->partitionTable(), 128);
	d->partitionTable()->updateUnallocated(*d);
	d->setIconName(QStringLiteral("drive-harddisk"));

	CoreBackend::setPartitionTableMaxPrimaries(*d->partitionTable(), 4);

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

#include "dummybackend.moc"
