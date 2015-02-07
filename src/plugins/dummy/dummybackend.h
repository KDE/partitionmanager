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

#if !defined(DUMMYBACKEND__H)

#define DUMMYBACKEND__H

#include "backend/corebackend.h"

#include <QList>
#include <QVariant>

class Device;
class KPluginFactory;
class QString;

/** Dummy backend plugin that doesn't really do anything.

	@author Volker Lanz <vl@fidra.de>
*/
class DummyBackend : public CoreBackend
{
	friend class KPluginFactory;

	Q_DISABLE_COPY(DummyBackend)

	private:
		DummyBackend(QObject* parent, const QList<QVariant>& args);

	public:
		virtual void initFSSupport();

		virtual QList<Device*> scanDevices();
		virtual CoreBackendDevice* openDevice(const QString& device_node);
		virtual CoreBackendDevice* openDeviceExclusive(const QString& device_node);
		virtual bool closeDevice(CoreBackendDevice* core_device);
		virtual Device* scanDevice(const QString& device_node);
};

#endif
