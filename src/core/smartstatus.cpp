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

#include "core/smartstatus.h"

#include <kdebug.h>

#include <QString>

#include <atasmart.h>
#include <errno.h>

SmartStatus::SmartStatus(const QString& device_path) :
	m_DevicePath(device_path),
	m_InitSuccess(false),
	m_Status(false),
	m_Temp(-99),
	m_BadSectors(-99),
	m_PowerCycles(-99),
	m_PoweredOn(-99)
{
	SkDisk* skDisk = NULL;
	SkBool skSmartStatus = false;
	uint64_t mkelvin = 0;
	uint64_t skBadSectors = 0;
	uint64_t skPoweredOn = 0;
	uint64_t skPowerCycles = 0;

	if (sk_disk_open(devicePath().toLocal8Bit(), &skDisk) < 0)
	{
		kDebug() << "smart disk open failed for " << devicePath() << ": " << strerror(errno);
		return;
	}

	if (sk_disk_smart_status(skDisk, &skSmartStatus) < 0)
	{
		kDebug() << "getting smart status failed for " << devicePath() << ": " << strerror(errno);
		sk_disk_free(skDisk);
		return;
	}

	setStatus(skSmartStatus);

	if (sk_disk_smart_read_data(skDisk) < 0)
	{
		kDebug() << "reading smart data failed for " << devicePath() << ": " << strerror(errno);
		sk_disk_free(skDisk);
		return;
	}

	if (sk_disk_smart_get_temperature(skDisk, &mkelvin) < 0)
		kDebug() << "getting temp failed for " <<  devicePath() << ": " << strerror(errno);
	else
		setTemp((mkelvin - 273150) / 100);

	if (sk_disk_smart_get_bad(skDisk, &skBadSectors) < 0)
		kDebug() << "getting bad sectors failed for " <<  devicePath() << ": " << strerror(errno);
	else
		setBadSectors(skBadSectors);

	if (sk_disk_smart_get_power_on(skDisk, &skPoweredOn) < 0)
		kDebug() << "getting powered on time failed for " <<  devicePath() << ": " << strerror(errno);
	else
		setPoweredOn(skPoweredOn);

	if (sk_disk_smart_get_power_cycle(skDisk, &skPowerCycles) < 0)
		kDebug() << "getting power cycles failed for " <<  devicePath() << ": " << strerror(errno);
	else
		setPowerCycles(skPowerCycles);

	sk_disk_free(skDisk);
	setInitSuccess(true);
}
