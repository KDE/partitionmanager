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

#include "core/devicescanner.h"

#include "backend/corebackend.h"

#include "core/operationstack.h"
#include "core/device.h"

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/block.h>
#include <solid/storagedrive.h>

/** Constructs a DeviceScanner
	@param ostack the OperationStack where the devices will be created
*/
DeviceScanner::DeviceScanner(QObject* parent, OperationStack& ostack) :
	QThread(parent),
	m_OperationStack(ostack)
{
}

void DeviceScanner::clear()
{
	operationStack().clearOperations();
	operationStack().clearDevices();
}

static quint32 countDevices(const QList<Solid::Device>& driveList)
{
	quint32 rval = 0;

	foreach(const Solid::Device& solidDevice, driveList)
	{
		const Solid::StorageDrive* solidDrive = solidDevice.as<Solid::StorageDrive>();
		if (solidDrive->driveType() == Solid::StorageDrive::HardDisk)
			rval++;
	}

	return rval;
}

void DeviceScanner::run()
{
	clear();

	const QList<Solid::Device> driveList = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive, QString());
	const quint32 totalDevices = countDevices(driveList);
	quint32 count = 0;

	foreach(const Solid::Device& solidDevice, driveList)
	{
		const Solid::StorageDrive* solidDrive = solidDevice.as<Solid::StorageDrive>();

		if (solidDrive->driveType() != Solid::StorageDrive::HardDisk)
			continue;

		const Solid::Block* solidBlock = solidDevice.as<Solid::Block>();

		Device* d = CoreBackend::self()->scanDevice(solidBlock->device());

		if (d != NULL)
		{
			d->setIconName(solidDevice.icon());
			operationStack().addDevice(d);
		}

		emit progress(solidBlock->device(), (++count) * 100 / totalDevices);
	}

	operationStack().sortDevices();
}
