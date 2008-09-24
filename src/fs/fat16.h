/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
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

#if !defined(FAT16__H)

#define FAT16__H

#include "fs/filesystem.h"

#include <qglobal.h>

class Partition;
class Report;

class QString;

namespace FS
{
	/** @brief A fat16 file system.
		@author vl@fidra.de
	 */
	class fat16 : public FileSystem
	{
		public:
			fat16(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type t = FileSystem::Fat16);

		public:
			static void init();

			virtual qint64 readUsedCapacity(const QString& deviceNode) const;
			virtual bool check(Report& report, const QString& deviceNode) const;
			virtual bool create(Report& report, const QString& deviceNode) const;
			virtual bool updateUUID(Report& report, const QString& deviceNode) const;

			virtual SupportType supportGetUsed() const { return m_GetUsed; }
			virtual SupportType supportCreate() const { return m_Create; }
			virtual SupportType supportGrow() const { return m_Grow; }
			virtual SupportType supportShrink() const { return m_Shrink; }
			virtual SupportType supportMove() const { return m_Move; }
			virtual SupportType supportCheck() const { return m_Check; }
			virtual SupportType supportCopy() const { return m_Copy; }
			virtual SupportType supportBackup() const { return m_Backup; }
			virtual SupportType supportUpdateUUID() const { return m_UpdateUUID; }
			
			virtual qint64 minCapacity() const;
			virtual qint64 maxCapacity() const;
			
		protected:
			static SupportType m_GetUsed;
			static SupportType m_Create;
			static SupportType m_Grow;
			static SupportType m_Shrink;
			static SupportType m_Move;
			static SupportType m_Check;
			static SupportType m_Copy;
			static SupportType m_Backup;
			static SupportType m_UpdateUUID;
	};
}

#endif
