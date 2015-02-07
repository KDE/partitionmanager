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

#include "core/devicescanner.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include "core/operationstack.h"
#include "core/device.h"

/** Constructs a DeviceScanner
	@param ostack the OperationStack where the devices will be created
*/
DeviceScanner::DeviceScanner(QObject* parent, OperationStack& ostack) :
	QThread(parent),
	m_OperationStack(ostack)
{
	setupConnections();
}

void DeviceScanner::setupConnections()
{
	connect(CoreBackendManager::self()->backend(), SIGNAL(scanProgress(QString,int)), SIGNAL(progress(QString,int)));
}

void DeviceScanner::clear()
{
	operationStack().clearOperations();
	operationStack().clearDevices();
}

void DeviceScanner::run()
{
	scan();
}

void DeviceScanner::scan()
{
	emit progress(QString(), 0);

	clear();

	QList<Device*> deviceList = CoreBackendManager::self()->backend()->scanDevices();

	foreach(Device* d, deviceList)
		operationStack().addDevice(d);

	operationStack().sortDevices();
}
