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

#if !defined(SIZEDIALOGBASE__H)

#define SIZEDIALOGBASE__H

#include "util/capacity.h"

#include <qglobal.h>
#include <qdebug.h>

#include <kdialog.h>

class Device;
class Partition;
class PartitionTable;
class SizeDialogWidget;

/** @brief Base class for all dialogs moving or resizing Partitions.
	@author vl@fidra.de
*/
class SizeDialogBase : public KDialog
{
	Q_OBJECT

	protected:
		SizeDialogBase(QWidget* parent, Capacity::Unit preferred, Device& device, Partition& part, qint64 freebefore, qint64 freeafter);
		virtual ~SizeDialogBase() {}

		SizeDialogWidget& dialogWidget() { Q_ASSERT(m_SizeDialogWidget); return *m_SizeDialogWidget; }
		const SizeDialogWidget& dialogWidget() const { Q_ASSERT(m_SizeDialogWidget); return *m_SizeDialogWidget; }

		virtual const PartitionTable& partitionTable() const;
		virtual bool canGrow() const { return true; }
		virtual bool canShrink() const { return true; }
		virtual bool canMove() const { return true; }
		virtual void setupDialog();
		virtual void setupConstraints();
		virtual void setupConnections();
		virtual Partition& partition() { return m_Partition; }
		virtual const Partition& partition() const { return m_Partition; }
		virtual Device& device() { return m_Device; }
		virtual const Device& device() const { return m_Device; }
		virtual qint64 minSectors() const;
		virtual qint64 maxSectors() const;
		virtual qint64 freeSectorsBefore() const { return m_FreeSectorsBefore; }
		virtual qint64 freeSectorsAfter() const { return m_FreeSectorsAfter; }
		virtual void setDirty() {}

		static int sectorsToDialogUnit(const Partition& p, Capacity::Unit u, qint64 v);
		static qint64 dialogUnitToSectors(const Partition& p, Capacity::Unit u, int v);

	protected slots:
		void onSectorsBeforeChanged(qint64 newBefore);
		void onSectorsAfterChanged(qint64 newAfter);
		void onLengthChanged(qint64 newLength);
		void onCapacityChanged(int newCapacity);
		void onFreeSpaceBeforeChanged(int newBefore);
		void onFreeSpaceAfterChanged(int newAfter);

	public:
		Capacity::Unit preferredUnit() const { return m_PreferredUnit; } /**< @return the preferred unit for a dialog */

	protected:
		SizeDialogWidget* m_SizeDialogWidget;
		Capacity::Unit m_PreferredUnit;
		Device& m_Device;
		Partition& m_Partition;
		qint64 m_FreeSectorsBefore;
		qint64 m_FreeSectorsAfter;
};

#endif
