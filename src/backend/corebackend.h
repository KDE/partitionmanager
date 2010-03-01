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

#if !defined(COREBACKEND__H)

#define COREBACKEND__H

#include "util/libpartitionmanagerexport.h"

#include <QObject>

class CoreBackendDevice;
class Device;
class QString;

class LIBPARTITIONMANAGERPRIVATE_EXPORT CoreBackend : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(CoreBackend)

	protected:
		CoreBackend() {}
		virtual ~CoreBackend() {}

	public:
		static CoreBackend* self();

		virtual CoreBackendDevice* openDevice(const QString& device_node) = 0;
		virtual CoreBackendDevice* openDeviceExclusive(const QString& device_node) = 0;

		virtual bool closeDevice(CoreBackendDevice* core_device) = 0;

		virtual QString about() const = 0;

	public:
		virtual Device* scanDevice(const QString& device_node) = 0;
};

#endif
