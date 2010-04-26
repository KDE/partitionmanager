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
#include "gui/sizedetailswidget.h"

#include "core/partitiontable.h"
#include "core/device.h"
#include "core/partition.h"
#include "core/partitionalignment.h"

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

static double sectorsToDialogUnit(const Partition& p, qint64 v)
{
	return Capacity(v * p.sectorSize()).toDouble(Capacity::preferredUnit());
}

static qint64 dialogUnitToSectors(const Partition& p, double v)
{
	return v * Capacity::unitFactor(Capacity::Byte, Capacity::preferredUnit()) / p.sectorSize();
}

void SizeDialogBase::setupDialog()
{
	dialogWidget().spinFreeBefore().setValue(sectorsToDialogUnit(partition(), partition().firstSector() - minimumFirstSector()));
	dialogWidget().spinFreeAfter().setValue(sectorsToDialogUnit(partition(), maximumLastSector() - partition().lastSector()));

	dialogWidget().spinCapacity().setValue(Capacity(partition().capacity()).toInt(Capacity::preferredUnit()));

	dialogWidget().spinFreeBefore().setSuffix(QString(" ") + Capacity::unitName(Capacity::preferredUnit()));
	dialogWidget().spinFreeAfter().setSuffix(QString(" ") + Capacity::unitName(Capacity::preferredUnit()));
	dialogWidget().spinCapacity().setSuffix(QString(" ") + Capacity::unitName(Capacity::preferredUnit()));

	detailsWidget().spinFirstSector().setValue(partition().firstSector());
	detailsWidget().spinLastSector().setValue(partition().lastSector());

	detailsWidget().checkAlign().setChecked(Config::alignDefault());

	dialogWidget().partResizerWidget().init(device(), partition(), minimumFirstSector(), maximumLastSector(), false, canMove());
	dialogWidget().partResizerWidget().setAlign(Config::alignDefault());
}

void SizeDialogBase::setupConstraints()
{
	dialogWidget().partResizerWidget().setMinimumLength(minimumLength());
	dialogWidget().partResizerWidget().setMaximumLength(maximumLength());

	dialogWidget().labelMinSize().setText(Capacity(minimumLength() * partition().sectorSize()).toString());
	dialogWidget().labelMaxSize().setText(Capacity(maximumLength() * partition().sectorSize()).toString());

	if (!canShrink() && !canGrow())
		dialogWidget().spinCapacity().setEnabled(false);

	dialogWidget().partResizerWidget().setMaximumFirstSector(maximumFirstSector());
	dialogWidget().partResizerWidget().setMinimumLastSector(minimumLastSector());

	const qint64 totalCapacity = sectorsToDialogUnit(partition(), maximumLastSector() - minimumFirstSector() + 1);

	const qint64 minCapacity = sectorsToDialogUnit(partition(), minimumLength());
	const qint64 maxCapacity = sectorsToDialogUnit(partition(), maximumLength());
	dialogWidget().spinCapacity().setRange(minCapacity, maxCapacity);

	const qint64 maxFree = totalCapacity - minCapacity;

	dialogWidget().spinFreeBefore().setRange(0, maxFree);
	dialogWidget().spinFreeAfter().setRange(0, maxFree);

	detailsWidget().spinFirstSector().setRange(minimumFirstSector(), maximumLastSector());
	detailsWidget().spinLastSector().setRange(minimumFirstSector(), maximumLastSector());

	onAlignToggled(align());
}

void SizeDialogBase::setupConnections()
{
	connect(&dialogWidget().partResizerWidget(), SIGNAL(firstSectorChanged(qint64)), SLOT(onFirstSectorChanged(qint64)));
	connect(&dialogWidget().partResizerWidget(), SIGNAL(lastSectorChanged(qint64)), SLOT(onLastSectorChanged(qint64)));

	connect(&dialogWidget().spinFreeBefore(), SIGNAL(valueChanged(double)), SLOT(onFreeSpaceBeforeChanged(double)));
	connect(&dialogWidget().spinFreeAfter(), SIGNAL(valueChanged(double)), SLOT(onFreeSpaceAfterChanged(double)));
	connect(&dialogWidget().spinCapacity(), SIGNAL(valueChanged(double)), SLOT(onCapacityChanged(double)));

	connect(&detailsWidget().spinFirstSector(), SIGNAL(valueChanged(double)), SLOT(onSpinFirstSectorChanged(double)));
	connect(&detailsWidget().spinLastSector(), SIGNAL(valueChanged(double)), SLOT(onSpinLastSectorChanged(double)));
	connect(&detailsWidget().checkAlign(), SIGNAL(toggled(bool)), SLOT(onAlignToggled(bool)));
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

void SizeDialogBase::onAlignToggled(bool align)
{
	dialogWidget().partResizerWidget().setAlign(align);

	detailsWidget().spinFirstSector().setSingleStep(align ? PartitionAlignment::sectorAlignment(device()) : 1);
	detailsWidget().spinLastSector().setSingleStep(align ? PartitionAlignment::sectorAlignment(device()) : 1);

	const double capacityStep = align ? sectorsToDialogUnit(partition(), PartitionAlignment::sectorAlignment(device())) : 1;

	dialogWidget().spinFreeBefore().setSingleStep(capacityStep);
	dialogWidget().spinFreeBefore().setSingleStep(capacityStep);
	dialogWidget().spinCapacity().setSingleStep(capacityStep);
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

/** Updates the Partition's length
	@param newLength the new length
	@return true on success
*/
void SizeDialogBase::onCapacityChanged(double newCapacity)
{
	qint64 newLength = dialogUnitToSectors(partition(), newCapacity);

	newLength = qBound(minimumLength(), newLength, qMin(maximumLastSector() - minimumFirstSector() + 1, maximumLength()));

	if (newLength == partition().length())
		return;

	const qint64 oldLength = partition().length();
// 	const qint64 oldCapacity = sectorsToDialogUnit(partition(), oldLength);

	qint64 delta = newLength - oldLength;

	qint64 tmp = qMin(delta, maximumLastSector() - partition().lastSector());
	delta -= tmp;

	if (tmp != 0)
	{
		qint64 newLastSector = partition().lastSector() + tmp;

		if (align())
			newLastSector = PartitionAlignment::alignedLastSector(device(), partition(), newLastSector, minimumLastSector(), maximumLastSector(), minimumLength(), maximumLength());

		dialogWidget().partResizerWidget().updateLastSector(newLastSector);
	}

	tmp = qMin(delta, partition().firstSector() - minimumFirstSector());
	delta -= tmp;

	if (tmp != 0)
	{
		qint64 newFirstSector = partition().firstSector() - tmp;

		if (align())
			newFirstSector = PartitionAlignment::alignedFirstSector(device(), partition(), newFirstSector, minimumFirstSector(), maximumFirstSector(), minimumLength(), maximumLength());

		dialogWidget().partResizerWidget().updateFirstSector(newFirstSector);
	}
}

void SizeDialogBase::onFreeSpaceBeforeChanged(double newBefore)
{
	qint64 newFirstSector = minimumFirstSector() + dialogUnitToSectors(partition(), newBefore);

	if (align())
	{
		const qint64 oldBefore = sectorsToDialogUnit(partition(), partition().firstSector() - minimumFirstSector());
		const qint64 deltaCorrection = newBefore > oldBefore ? PartitionAlignment::firstDelta(device(), partition(), newFirstSector) : 0;
		newFirstSector = PartitionAlignment::alignedFirstSector(device(), partition(), newFirstSector + deltaCorrection, minimumFirstSector(), -1, minimumLength(), maximumLength());
	}

	if (dialogWidget().partResizerWidget().movePartition(newFirstSector) ||
			dialogWidget().partResizerWidget().updateFirstSector(newFirstSector))
		setDirty();
}

void SizeDialogBase::onFreeSpaceAfterChanged(double newAfter)
{
	qint64 newLastSector = maximumLastSector() - dialogUnitToSectors(partition(), newAfter);

	if (align())
	{
		const double oldAfter = sectorsToDialogUnit(partition(), maximumLastSector() - partition().lastSector());
		const qint64 deltaCorrection = newAfter > oldAfter ? PartitionAlignment::lastDelta(device(), partition(), newLastSector) : 0;
		newLastSector = PartitionAlignment::alignedLastSector(device(), partition(), newLastSector - deltaCorrection, -1, maximumLastSector(), minimumLength(), maximumLength());
	}

	const qint64 newFirstSector = newLastSector - partition().length() + 1;

	if (dialogWidget().partResizerWidget().movePartition(newFirstSector) ||
			dialogWidget().partResizerWidget().updateLastSector(newLastSector))
		setDirty();
}

const PartitionTable& SizeDialogBase::partitionTable() const
{
	Q_ASSERT(device().partitionTable());

	return *device().partitionTable();
}

bool SizeDialogBase::align() const
{
	return detailsWidget().checkAlign().isChecked();
}

qint64 SizeDialogBase::minimumLastSector() const
{
	return partition().minLastSector();
}

qint64 SizeDialogBase::maximumFirstSector() const
{
	return partition().maxFirstSector();
}

