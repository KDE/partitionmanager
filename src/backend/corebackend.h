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
#include <QList>

class CoreBackendManager;
class CoreBackendDevice;
class Device;
class PartitionTable;

class KAboutData;

class QString;

class LIBPARTITIONMANAGERPRIVATE_EXPORT CoreBackend : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(CoreBackend)

	friend class CoreBackendManager;

	protected:
		CoreBackend();
		virtual ~CoreBackend();

	signals:
		void progress(int);
		void scanProgress(const QString&,int);

	public:
		virtual const KAboutData& about() const { return *m_AboutData; }

		virtual void initFSSupport() = 0;

		virtual QList<Device*> scanDevices() = 0;
		virtual Device* scanDevice(const QString& device_node) = 0;
		virtual CoreBackendDevice* openDevice(const QString& device_node) = 0;
		virtual CoreBackendDevice* openDeviceExclusive(const QString& device_node) = 0;
		virtual bool closeDevice(CoreBackendDevice* core_device) = 0;
		virtual void emitProgress(int i);
		virtual void emitScanProgress(const QString& device_node, int i);

	protected:
		static void setPartitionTableForDevice(Device& d, PartitionTable* p);
		static void setPartitionTableMaxPrimaries(PartitionTable& p, qint32 max_primaries);

	private:
		void setAboutData(const KAboutData* a) { m_AboutData = a; }

	private:
		const KAboutData* m_AboutData;

		class CoreBackendPrivate;
		CoreBackendPrivate* d;
};

#endif
