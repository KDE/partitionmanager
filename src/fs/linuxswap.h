/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
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

#if !defined(LINUXSWAP__H)

#define LINUXSWAP__H

#include "fs/filesystem.h"

#include <qglobal.h>

class Report;

class QString;

namespace FS
{
	/** @brief A linux swap pseudo file system.
		@author vl@fidra.de
	 */
	class linuxswap : public FileSystem
	{
		public:
			linuxswap(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

		public:
			static void init();

			virtual bool create(Report& report, const QString& deviceNode) const;
			virtual bool resize(Report& report, const QString& deviceNode, qint64 length) const;
			virtual bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel);

			virtual bool canMount(const QString&) const { return true; }
			virtual bool canUnmount(const QString&) const { return true; }

			virtual bool mount(const QString& deviceNode);
			virtual bool unmount(const QString& deviceNode);

			virtual QString mountTitle() const;
			virtual QString unmountTitle() const;

			virtual SupportType supportCreate() const { return m_Create; }
			virtual SupportType supportGrow() const { return m_Grow; }
			virtual SupportType supportShrink() const { return m_Shrink; }
			virtual SupportType supportMove() const { return m_Move; }
			virtual SupportType supportCopy() const { return m_Copy; }
			virtual SupportType supportGetLabel() const { return m_GetLabel; }
			virtual SupportType supportSetLabel() const { return m_SetLabel; }
			virtual SupportType supportGetUUID() const { return m_GetUUID; }

		protected:
			static SupportType m_Create;
			static SupportType m_Grow;
			static SupportType m_Shrink;
			static SupportType m_Move;
			static SupportType m_Copy;
			static SupportType m_SetLabel;
			static SupportType m_GetLabel;
			static SupportType m_GetUUID;
	};
}

#endif
