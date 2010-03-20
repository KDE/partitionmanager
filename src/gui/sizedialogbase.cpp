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

#include "gui/sizedialogbase.h"
#include "gui/sizedetailswidget.h"
#include "gui/partresizerwidget.h"
#include "gui/sizedialogwidget.h"

#include "core/partitiontable.h"
#include "core/device.h"
#include "core/partition.h"

#include "util/capacity.h"

#include <kdebug.h>
#include <config.h>

SizeDialogBase::SizeDialogBase(QWidget* parent, Device& d, Partition& part, qint64 minFirst, qint64 maxLast) :
	KDialog(parent),
	m_SizeDialogWidget(new SizeDialogWidget(this)),
	m_SizeDetailsWidget(new SizeDetailsWidget(this)),
	m_Device(d),
	m_Partition(part),
	m_MinimumFirstSector(minFirst),
	m_MaximumLastSector(maxLast)
{
	setMainWidget(&dialogWidget());
	setDetailsWidget(&detailsWidget());

	showButtonSeparator(true);
	setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Details);
	// Cannot use KGuiItem() for the details button due to a KDialog bug -- it has special handling
	// for the details button text but not if using setButtonGuiItem
	setButtonText(Details, i18nc("@item:button advanced settings button", "Advanced"));
	setButtonIcon(Details, KIcon());
}

qint64 SizeDialogBase::minimumLength() const
{
	if (!canShrink())
		return partition().length();

	return qMax(partition().sectorsUsed(), partition().minimumSectors());
}

qint64 SizeDialogBase::maximumLength() const
{
	if (!canGrow())
		return partition().length();

	return qMin(maximumLastSector() - minimumFirstSector() + 1, partition().maximumSectors());
}

static int sectorsToDialogUnit(const Partition& p, qint64 v)
{
	return Capacity(v * p.sectorSize()).toInt(Capacity::preferredUnit());
}

static qint64 dialogUnitToSectors(const Partition& p, int v)
{
	return Capacity::unitFactor(Capacity::Byte, Capacity::preferredUnit()) * v / p.sectorSize();
}

void SizeDialogBase::setupDialog()
{
	dialogWidget().partResizerWidget().init(device(), partition(), minimumFirstSector(), maximumLastSector());

	// TODO: these don't belong here; the distinction between setupDialog and setupConstraints
	// doesn't work that well, there's too much interdependency.
	if (!canShrink())
		dialogWidget().partResizerWidget().setMinimumLength(partition().length());

	if (!canGrow())
		dialogWidget().partResizerWidget().setMaximumLength(partition().length());

	dialogWidget().spinFreeBefore().setValue(sectorsToDialogUnit(partition(), partition().firstSector() - minimumFirstSector()));
	dialogWidget().spinFreeAfter().setValue(sectorsToDialogUnit(partition(), maximumLastSector() - partition().lastSector()));

	dialogWidget().spinCapacity().setValue(Capacity(partition().capacity()).toInt(Capacity::preferredUnit()));

	dialogWidget().spinFreeBefore().setSuffix(QString(" ") + Capacity::unitName(Capacity::preferredUnit()));
	dialogWidget().spinFreeAfter().setSuffix(QString(" ") + Capacity::unitName(Capacity::preferredUnit()));
	dialogWidget().spinCapacity().setSuffix(QString(" ") + Capacity::unitName(Capacity::preferredUnit()));

	detailsWidget().spinFirstSector().setValue(partition().firstSector());
	detailsWidget().spinLastSector().setValue(partition().lastSector());
}

void SizeDialogBase::setupConstraints()
{
	dialogWidget().partResizerWidget().setMinimumLength(minimumLength());
	dialogWidget().partResizerWidget().setMaximumLength(maximumLength());

	dialogWidget().labelMinSize().setText(Capacity(minimumLength() * partition().sectorSize()).toString());
	dialogWidget().labelMaxSize().setText(Capacity(maximumLength() * partition().sectorSize()).toString());

	if (!canShrink() && !canGrow())
		dialogWidget().spinCapacity().setEnabled(false);

	if (!canMove())
		dialogWidget().partResizerWidget().setMoveAllowed(false);

	dialogWidget().partResizerWidget().setMaximumFirstSector(partition().maxFirstSector());
	dialogWidget().partResizerWidget().setMinimumLastSector(partition().minLastSector());

	const qint64 totalCapacity = sectorsToDialogUnit(partition(), maximumLastSector() - minimumFirstSector() + 1);

	const qint64 minCapacity = sectorsToDialogUnit(partition(), minimumLength());
	const qint64 maxCapacity = sectorsToDialogUnit(partition(), maximumLength());
	dialogWidget().spinCapacity().setRange(minCapacity, maxCapacity);

	const qint64 maxFree = totalCapacity - minCapacity;

	dialogWidget().spinFreeBefore().setRange(0, maxFree);
	dialogWidget().spinFreeAfter().setRange(0, maxFree);

	detailsWidget().spinFirstSector().setRange(minimumFirstSector(), maximumLastSector());
	detailsWidget().spinLastSector().setRange(minimumFirstSector(), maximumLastSector());

	detailsWidget().spinFirstSector().setSingleStep(Config::sectorAlignment());
	detailsWidget().spinLastSector().setSingleStep(Config::sectorAlignment());
}

void SizeDialogBase::setupConnections()
{
	connect(&dialogWidget().partResizerWidget(), SIGNAL(firstSectorChanged(qint64)), SLOT(onFirstSectorChanged(qint64)));
	connect(&dialogWidget().partResizerWidget(), SIGNAL(lastSectorChanged(qint64)), SLOT(onLastSectorChanged(qint64)));

	connect(&dialogWidget().spinFreeBefore(), SIGNAL(valueChanged(int)), SLOT(onFreeSpaceBeforeChanged(int)));
	connect(&dialogWidget().spinFreeAfter(), SIGNAL(valueChanged(int)), SLOT(onFreeSpaceAfterChanged(int)));
	connect(&dialogWidget().spinCapacity(), SIGNAL(valueChanged(int)), SLOT(onCapacityChanged(int)));

	connect(&detailsWidget().spinFirstSector(), SIGNAL(valueChanged(double)), SLOT(onSpinFirstSectorChanged(double)));
	connect(&detailsWidget().spinLastSector(), SIGNAL(valueChanged(double)), SLOT(onSpinLastSectorChanged(double)));
	connect(&detailsWidget().checkAlign(), SIGNAL(stateChanged(int)), SLOT(onAlignStateChanged(int)));

}

void SizeDialogBase::onSpinFirstSectorChanged(double newFirst)
{
	if (newFirst >= minimumFirstSector())
	{
		dialogWidget().partResizerWidget().updateFirstSector(newFirst);
		setDirty();
	}
}

void SizeDialogBase::onSpinLastSectorChanged(double newLast)
{
	if (newLast <= maximumLastSector())
	{
		dialogWidget().partResizerWidget().updateLastSector(newLast);
		setDirty();
	}
}

void SizeDialogBase::onAlignStateChanged(int)
{
	const bool align = detailsWidget().checkAlign().isChecked();
	dialogWidget().partResizerWidget().setAlign(align);
	detailsWidget().spinFirstSector().setSingleStep(align ? Config::sectorAlignment() : 1);
	detailsWidget().spinLastSector().setSingleStep(align ? Config::sectorAlignment() : 1);
}

void SizeDialogBase::onFirstSectorChanged(qint64 newFirst)
{
	bool state = dialogWidget().spinFreeBefore().blockSignals(true);
	dialogWidget().spinFreeBefore().setValue(sectorsToDialogUnit(partition(), newFirst - minimumFirstSector()));
	dialogWidget().spinFreeBefore().blockSignals(state);

	state = detailsWidget().spinFirstSector().blockSignals(true);
	detailsWidget().spinFirstSector().setValue(newFirst);
	detailsWidget().spinFirstSector().blockSignals(state);

	updateLength(partition().length());
	setDirty();
}

void SizeDialogBase::onLastSectorChanged(qint64 newLast)
{
	bool state = dialogWidget().spinFreeAfter().blockSignals(true);
	dialogWidget().spinFreeAfter().setValue(sectorsToDialogUnit(partition(), maximumLastSector() - newLast));
	dialogWidget().spinFreeAfter().blockSignals(state);

	state = detailsWidget().spinLastSector().blockSignals(true);
	detailsWidget().spinLastSector().setValue(newLast);
	detailsWidget().spinLastSector().blockSignals(state);

	updateLength(partition().length());
	setDirty();
}

void SizeDialogBase::updateLength(qint64 newLength)
{
	bool state = dialogWidget().spinCapacity().blockSignals(true);
	dialogWidget().spinCapacity().setValue(sectorsToDialogUnit(partition(), newLength));
	dialogWidget().spinCapacity().blockSignals(state);
}

void SizeDialogBase::onCapacityChanged(int newCapacity)
{
	const qint64 newLength = dialogUnitToSectors(partition(), newCapacity);
	dialogWidget().partResizerWidget().updateLength(newLength);
}

void SizeDialogBase::onFreeSpaceBeforeChanged(int newBefore)
{
	const qint64 newFirstSector = minimumFirstSector() + dialogUnitToSectors(partition(), newBefore);
	if (!dialogWidget().partResizerWidget().movePartition(newFirstSector))
		dialogWidget().partResizerWidget().updateFirstSector(newFirstSector);
	setDirty();
}

void SizeDialogBase::onFreeSpaceAfterChanged(int newAfter)
{
	const qint64 newLastSector = maximumLastSector() - dialogUnitToSectors(partition(), newAfter);
	const qint64 newFirstSector = newLastSector - partition().length() + 1;
	if (!dialogWidget().partResizerWidget().movePartition(newFirstSector))
		dialogWidget().partResizerWidget().updateLastSector(newLastSector);
	setDirty();
}

const PartitionTable& SizeDialogBase::partitionTable() const
{
	Q_ASSERT(device().partitionTable());

	return *device().partitionTable();
}
