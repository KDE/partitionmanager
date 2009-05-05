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

#include "gui/sizedialogbase.h"
#include "gui/partresizerwidget.h"
#include "gui/sizedialogwidget.h"

#include "core/partitiontable.h"
#include "core/device.h"
#include "core/partition.h"


#include <kdebug.h>

SizeDialogBase::SizeDialogBase(QWidget* parent, Capacity::Unit preferred, Device& device, Partition& part, qint64 freebefore, qint64 freeafter) :
	KDialog(parent),
	m_SizeDialogWidget(new SizeDialogWidget(this)),
	m_PreferredUnit(preferred),
	m_Device(device),
	m_Partition(part),
	m_FreeSectorsBefore(freebefore),
	m_FreeSectorsAfter(freeafter)
{
}

qint64 SizeDialogBase::minSectors() const
{
	if (!canShrink())
		return partition().length();

	return qMax(partition().sectorsUsed(), partition().minimumSectors());
}

qint64 SizeDialogBase::maxSectors() const
{
	if (!canGrow())
		return partition().length();

	return qMin(partition().length() + freeSectorsBefore() + freeSectorsAfter(), partition().maximumSectors());
}

int SizeDialogBase::sectorsToDialogUnit(const Partition& p, Capacity::Unit u, qint64 v)
{
	return Capacity(v * p.sectorSize()).toInt(u);
}

qint64 SizeDialogBase::dialogUnitToSectors(const Partition& p, Capacity::Unit u, int v)
{
	return Capacity::unitFactor(Capacity::Byte, u) * v / p.sectorSize();
}

void SizeDialogBase::setupDialog()
{
	dialogWidget().spinFreeBefore().setValue(Capacity(freeSectorsBefore() * partition().sectorSize()).toInt(preferredUnit()));
	dialogWidget().spinFreeAfter().setValue(Capacity(freeSectorsAfter() * partition().sectorSize()).toInt(preferredUnit()));
	dialogWidget().spinCapacity().setValue(Capacity(partition().capacity()).toInt(preferredUnit()));

	dialogWidget().spinFreeBefore().setSuffix(QString(" ") + Capacity::unitName(preferredUnit()));
	dialogWidget().spinFreeAfter().setSuffix(QString(" ") + Capacity::unitName(preferredUnit()));
	dialogWidget().spinCapacity().setSuffix(QString(" ") + Capacity::unitName(preferredUnit()));

	dialogWidget().partResizerWidget().init(device(), partition(), freeSectorsBefore(), freeSectorsAfter());
}

void SizeDialogBase::setupConstraints()
{
	dialogWidget().partResizerWidget().setMinimumSectors(minSectors());
	dialogWidget().partResizerWidget().setMaximumSectors(maxSectors());

	dialogWidget().labelMinSize().setText(Capacity(minSectors() * partition().sectorSize()).toString());
	dialogWidget().labelMaxSize().setText(Capacity(maxSectors() * partition().sectorSize()).toString());

	if (!canShrink() && !canGrow())
		dialogWidget().spinCapacity().setEnabled(false);

	if (!canMove())
		dialogWidget().partResizerWidget().setMoveAllowed(false);

	dialogWidget().partResizerWidget().setMaxFirstSector(partition().maxFirstSector());
	dialogWidget().partResizerWidget().setMinLastSector(partition().minLastSector());

	const qint64 totalCapacity = sectorsToDialogUnit(partition(), preferredUnit(), dialogWidget().partResizerWidget().totalSectors());

	const qint64 minCapacity = sectorsToDialogUnit(partition(), preferredUnit(), minSectors());
	const qint64 maxCapacity = sectorsToDialogUnit(partition(), preferredUnit(), maxSectors());
	dialogWidget().spinCapacity().setRange(minCapacity, maxCapacity);

	const qint64 maxFree = totalCapacity - minCapacity;

	dialogWidget().spinFreeBefore().setRange(0, maxFree);
	dialogWidget().spinFreeAfter().setRange(0, maxFree);
}

void SizeDialogBase::setupConnections()
{
	connect(&dialogWidget().partResizerWidget(), SIGNAL(sectorsBeforeChanged(qint64)), SLOT(onSectorsBeforeChanged(qint64)));
	connect(&dialogWidget().partResizerWidget(), SIGNAL(sectorsAfterChanged(qint64)), SLOT(onSectorsAfterChanged(qint64)));
	connect(&dialogWidget().partResizerWidget(), SIGNAL(lengthChanged(qint64)), SLOT(onLengthChanged(qint64)));

	connect(&dialogWidget().spinFreeBefore(), SIGNAL(valueChanged(int)), SLOT(onFreeSpaceBeforeChanged(int)));
	connect(&dialogWidget().spinFreeAfter(), SIGNAL(valueChanged(int)), SLOT(onFreeSpaceAfterChanged(int)));
	connect(&dialogWidget().spinCapacity(), SIGNAL(valueChanged(int)), SLOT(onCapacityChanged(int)));
}

void SizeDialogBase::onSectorsBeforeChanged(qint64 newBefore)
{
	dialogWidget().spinFreeBefore().disconnect(this);
	dialogWidget().spinFreeBefore().setValue(sectorsToDialogUnit(partition(), preferredUnit(), newBefore));
	connect(&dialogWidget().spinFreeBefore(), SIGNAL(valueChanged(int)), SLOT(onFreeSpaceBeforeChanged(int)));
	setFreeSectorsBefore(newBefore);
	setDirty();
}

void SizeDialogBase::onSectorsAfterChanged(qint64 newAfter)
{
	dialogWidget().spinFreeAfter().disconnect(this);
	dialogWidget().spinFreeAfter().setValue(sectorsToDialogUnit(partition(), preferredUnit(), newAfter));
	connect(&dialogWidget().spinFreeAfter(), SIGNAL(valueChanged(int)), SLOT(onFreeSpaceAfterChanged(int)));
	setFreeSectorsAfter(newAfter);
	setDirty();
}

void SizeDialogBase::onLengthChanged(qint64 newLength)
{
	dialogWidget().spinCapacity().disconnect(this);
	dialogWidget().spinCapacity().setValue(sectorsToDialogUnit(partition(), preferredUnit(), newLength));
	connect(&dialogWidget().spinCapacity(), SIGNAL(valueChanged(int)), SLOT(onCapacityChanged(int)));
}

void SizeDialogBase::onCapacityChanged(int newCapacity)
{
	qint64 newLength = dialogUnitToSectors(partition(), preferredUnit(), newCapacity);
	dialogWidget().partResizerWidget().updateLength(newLength);
}

void SizeDialogBase::onFreeSpaceBeforeChanged(int newBefore)
{
	qint64 newSectorsBefore = dialogUnitToSectors(partition(), preferredUnit(), newBefore);
	qint64 delta = dialogWidget().partResizerWidget().sectorsBefore() - newSectorsBefore;
	qint64 newSectorsAfter = dialogWidget().partResizerWidget().sectorsAfter() + delta;

	if (newSectorsAfter < 0)
	{
		dialogWidget().partResizerWidget().updateLength(partition().length() + newSectorsAfter);
		newSectorsAfter = 0;
	}

	dialogWidget().partResizerWidget().updateSectors(newSectorsBefore, newSectorsAfter);
	setDirty();
}

void SizeDialogBase::onFreeSpaceAfterChanged(int newAfter)
{
	qint64 newSectorsAfter = dialogUnitToSectors(partition(), preferredUnit(), newAfter);
	qint64 delta = newSectorsAfter - dialogWidget().partResizerWidget().sectorsAfter();
	qint64 newSectorsBefore = dialogWidget().partResizerWidget().sectorsBefore() - delta;

	if (newSectorsBefore < 0)
	{
		dialogWidget().partResizerWidget().updateLength(partition().length() + newSectorsBefore);
		newSectorsBefore = 0;
	}

	dialogWidget().partResizerWidget().updateSectors(newSectorsBefore, newSectorsAfter);
	setDirty();
}

const PartitionTable& SizeDialogBase::partitionTable() const
{
	Q_ASSERT(device().partitionTable());

	return *device().partitionTable();
}
