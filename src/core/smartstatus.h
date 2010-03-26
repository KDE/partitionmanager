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

#if !defined(SMARTSTATUS__H)

#define SMARTSTATUS__H

#include <qglobal.h>
#include <QString>

class SmartStatus
{
	public:
		SmartStatus(const QString& device_path);

	public:
		const QString& devicePath() const { return m_DevicePath; }
		bool isValid() const { return m_InitSuccess; }
		bool status() const { return m_Status; }
		qint32 temp() const { return m_Temp; }
		qint64 badSectors() const { return m_BadSectors; }
		qint64 powerCycles() const { return m_PowerCycles; }
		qint64 poweredOn() const { return m_PoweredOn; }

	protected:
		void setStatus(bool s) { m_Status = s; }
		void setTemp(qint32 t) { m_Temp = t; }
		void setInitSuccess(bool b) { m_InitSuccess = b; }
		void setBadSectors(qint64 s) { m_BadSectors = s; }
		void setPowerCycles(qint64 p) { m_PowerCycles = p; }
		void setPoweredOn(qint64 t) { m_PoweredOn = t; }

	private:
		const QString m_DevicePath;
		bool m_InitSuccess;
		bool m_Status;
		qint32 m_Temp;
		qint64 m_BadSectors;
		qint64 m_PowerCycles;
		qint64 m_PoweredOn;
};

#endif


