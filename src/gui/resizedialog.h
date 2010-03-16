/***************************************************************************
 *   Copyright (C) 2008,2010 by Volker Lanz <vl@fidra.de>                  *
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

#if !defined(RESIZEDIALOG__H)

#define RESIZEDIALOG__H

#include "gui/sizedialogbase.h"

#include <qglobal.h>

class Partition;
class Device;

/** @brief Let the user resize or move a Partition.

	@todo The ResizeDialog does not yet have a "dirty bit".
	It should get one and disable its OK button as long as nothing has been
	modified, much like the PartPropsDialog already does.

	@author vl@fidra.de
*/
class ResizeDialog : public SizeDialogBase
{
	Q_OBJECT

	public:
		ResizeDialog(QWidget* parent, Device& device, Partition& p, qint64 minFirst, qint64 maxLast);
		~ResizeDialog();

	public:
		bool isModified() const;

	protected:
		virtual bool canGrow() const;
		virtual bool canShrink() const;
		virtual bool canMove() const;
		virtual void setupDialog();
		virtual void setDirty();

	protected:
		qint64 originalFirstSector() const { return m_OriginalFirstSector; }
		qint64 originalLastSector() const { return m_OriginalLastSector; }

	private:
		qint64 m_OriginalFirstSector;
		qint64 m_OriginalLastSector;
};

#endif
