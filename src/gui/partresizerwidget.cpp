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

#include "gui/partresizerwidget.h"
#include "gui/partwidget.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/partitiontable.h"

#include "fs/filesystem.h"

#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>

#include <kdebug.h>
#include <kcolorscheme.h>

const qint32 PartResizerWidget::m_HandleWidth = 16;
const qint32 PartResizerWidget::m_HandleHeight = 59;

/** Creates a new PartResizerWidget

	Initializing is mostly done in init().

	@param parent pointer to the parent widget
*/
PartResizerWidget::PartResizerWidget(QWidget* parent) :
	QWidget(parent),
	m_Device(NULL),
	m_Partition(NULL),
	m_PartWidget(NULL),
	m_MinimumFirstSector(0),
	m_MaximumFirstSector(-1),
	m_MinimumLastSector(-1),
	m_MaximumLastSector(0),
	m_MinimumLength(-1),
	m_MaximumLength(-1),
	m_LeftHandle(this),
	m_RightHandle(this),
	m_DraggedWidget(NULL),
	m_Hotspot(0),
	m_MoveAllowed(true),
	m_ReadOnly(false),
	m_Align(true)
{
}

/** Intializes the PartResizerWidget
	@param d the Device the Partition is on
	@param p the Partition to show and/or resize
	@param minFirst the minimum value for the first sector
	@param maxLast the maximum value for the last sector
*/
void PartResizerWidget::init(Device& d, Partition& p, qint64 minFirst, qint64 maxLast)
{
	setDevice(d);
	setPartition(p);

	setMinimumFirstSector(minFirst);
	setMaximumLastSector(maxLast);

	setMinimumLength(qMax(partition().sectorsUsed(), partition().minimumSectors()));
	setMaximumLength(qMin(totalSectors(), partition().maximumSectors()));

	/** @todo get real pixmaps for the handles */
	QPixmap pixmap(handleWidth(), handleHeight());
	pixmap.fill(QColor(0x44, 0x44, 0x44));

	leftHandle().setPixmap(pixmap);
	rightHandle().setPixmap(pixmap);
	leftHandle().setFixedSize(handleWidth(), handleHeight());
	rightHandle().setFixedSize(handleWidth(), handleHeight());

	delete m_PartWidget;
	m_PartWidget = new PartWidget(this, NULL, &partition());

	if (!readOnly())
	{
		leftHandle().setCursor(Qt::SizeHorCursor);
		rightHandle().setCursor(Qt::SizeHorCursor);

		if (moveAllowed())
			partWidget().setCursor(Qt::SizeAllCursor);

		partWidget().setToolTip(QString());
	}

	updatePositions();
}

qint64 PartResizerWidget::sectorsPerPixel() const
{
	return totalSectors() / (width() - 2 * handleWidth());
}

int PartResizerWidget::partWidgetStart() const
{
	return handleWidth() + (partition().firstSector() - minimumFirstSector()) / sectorsPerPixel();
}

int PartResizerWidget::partWidgetWidth() const
{
	return partition().length() / sectorsPerPixel();
}

void PartResizerWidget::updatePositions()
{
	partWidget().move(partWidgetStart(), 0);
	partWidget().resize(partWidgetWidth(), height() - 1);
	leftHandle().move(partWidgetStart() - leftHandle().width(), 0);
	rightHandle().move(partWidgetStart() + partWidgetWidth(), 0);

	partWidget().update();
}

void PartResizerWidget::resizeEvent(QResizeEvent* event)
{
	updatePositions();
	QWidget::resizeEvent(event);
}

void PartResizerWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor(0x99, 0x99, 0x99));
	painter.drawRect(QRect(handleWidth(), 0, width() - (2 * handleWidth()) - 1, height() - 1));
}

void PartResizerWidget::mousePressEvent(QMouseEvent* event)
{
	if (readOnly())
		return;

	if (event->button() == Qt::LeftButton)
	{
		m_DraggedWidget = static_cast<QWidget*>(childAt(event->pos()));

		if (m_DraggedWidget != NULL)
		{
			if (partWidget().isAncestorOf(m_DraggedWidget))
				m_DraggedWidget = &partWidget();

			m_Hotspot = m_DraggedWidget->mapFromParent(event->pos()).x();
		}
	}
}

bool PartResizerWidget::movePartition(qint64 newFirstSector)
{
	if (maximumFirstSector() > -1 && newFirstSector > maximumFirstSector())
		newFirstSector = maximumFirstSector();

	if (minimumFirstSector() > -1 && newFirstSector < minimumFirstSector())
		newFirstSector = minimumFirstSector();

	qint64 delta = newFirstSector - partition().firstSector();
	qint64 newLastSector = partition().lastSector() + delta;

	if (minimumLastSector() > -1 && newLastSector < minimumLastSector())
	{
		const qint64 deltaLast = minimumLastSector() - newLastSector;
		newFirstSector += deltaLast;
		newLastSector += deltaLast;
	}

	if (maximumLastSector() > -1 && newLastSector > maximumLastSector())
	{
		const qint64 deltaLast = newLastSector - maximumLastSector();
		newFirstSector -= deltaLast;
		newLastSector -= deltaLast;
	}

	if (newLastSector - newFirstSector + 1 != partition().length() ||
			(maximumFirstSector() > -1 && newFirstSector > maximumFirstSector()) ||
			(minimumFirstSector() > -1 && newFirstSector < minimumFirstSector()) ||
			(minimumLastSector() > -1 && newLastSector < minimumLastSector()) ||
			(maximumLastSector() > -1 && newLastSector > maximumLastSector()))
	{
		kWarning() << "constraints not satisfied while trying to move partition " << partition().deviceNode();
		return false;
	}

	if (partition().children().size() > 0 &&
		(!checkAlignment(*partition().children().first(), partition().firstSector() - newFirstSector) ||
		!checkAlignment(*partition().children().last(), partition().lastSector() - newLastSector)))
	{
		kDebug() << "cannot align children while trying to move partition " << partition().deviceNode();
		return false;
	}

	const qint64 originalFirst = partition().firstSector();
	const qint64 originalLast = partition().lastSector();

	partition().setFirstSector(newFirstSector);
	partition().fileSystem().setFirstSector(newFirstSector);

	partition().setLastSector(newLastSector);
	partition().fileSystem().setLastSector(newLastSector);

	if (align())
		device().partitionTable()->alignPartition(device(), partition());

	if (originalFirst != partition().firstSector() || originalLast != partition().lastSector())
	{
		resizeLogicals();
		updatePositions();
	}

	if (originalFirst != partition().firstSector())
		emit firstSectorChanged(partition().firstSector());

	if (originalLast != partition().lastSector())
		emit lastSectorChanged(partition().lastSector());

	return originalFirst != partition().firstSector() || originalLast != partition().lastSector();
}

void PartResizerWidget::mouseMoveEvent(QMouseEvent* event)
{
	int x = event->pos().x() - m_Hotspot;

	if (draggedWidget() == &leftHandle())
	{
		const qint64 newFirstSector = qMax(minimumFirstSector() + x * sectorsPerPixel(), 0LL);
		updateFirstSector(newFirstSector);
	}
	else if (draggedWidget() == &rightHandle())
	{
		const qint64 newLastSector = qMin(minimumFirstSector() + (x - rightHandle().width()) * sectorsPerPixel(), maximumLastSector());
		updateLastSector(newLastSector);
	}
	else if (draggedWidget() == &partWidget() && moveAllowed())
	{
		const qint64 newFirstSector = qMax(minimumFirstSector() + (x - handleWidth()) * sectorsPerPixel(), 0LL);
		movePartition(newFirstSector);
	}
}

void PartResizerWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		m_DraggedWidget = NULL;
}

bool PartResizerWidget::updateFirstSector(qint64 newFirstSector)
{
	if (maximumFirstSector() > -1 && newFirstSector > maximumFirstSector())
		newFirstSector = maximumFirstSector();

	if (minimumFirstSector() > -1 && newFirstSector < minimumFirstSector())
		newFirstSector = minimumFirstSector();

	const qint64 newLength = partition().lastSector() - newFirstSector + 1;

	if (newLength < minimumLength())
		newFirstSector -= minimumLength() - newLength;

	if (newLength > maximumLength())
		newFirstSector -= newLength - maximumLength();

	if (newFirstSector != partition().firstSector() && (partition().children().size() == 0 || checkAlignment(*partition().children().first(), partition().firstSector() - newFirstSector)))
	{
		const qint64 originalFirst = partition().lastSector();

		partition().setFirstSector(newFirstSector);
		partition().fileSystem().setFirstSector(newFirstSector);

		if (align())
			device().partitionTable()->alignPartition(device(), partition());

		if (originalFirst != partition().firstSector())
		{
			resizeLogicals();
			updatePositions();
			emit firstSectorChanged(partition().firstSector());
			return true;
		}
	}

	return false;
}

bool PartResizerWidget::checkAlignment(const Partition& child, qint64 delta) const
{
	if (!partition().roles().has(PartitionRole::Extended))
		return true;

	if (child.roles().has(PartitionRole::Unallocated))
		return true;

	return qAbs(delta) >= PartitionTable::sectorAlignment(device());
}

void PartResizerWidget::resizeLogicals()
{
	if (!partition().roles().has(PartitionRole::Extended) || partition().children().size() == 0)
		return;

	Q_ASSERT(device().partitionTable());

	device().partitionTable()->removeUnallocated(&partition());
	device().partitionTable()->insertUnallocated(device(), &partition(), partition().firstSector());

	partWidget().updateChildren();
}

bool PartResizerWidget::updateLastSector(qint64 newLastSector)
{
	if (minimumLastSector() > -1 && newLastSector < minimumLastSector())
		newLastSector = minimumLastSector();

	if (maximumLastSector() > -1 && newLastSector > maximumLastSector())
		newLastSector = maximumLastSector();

	const qint64 newLength = newLastSector - partition().firstSector() + 1;

	if (newLength < minimumLength())
		newLastSector += minimumLength() - newLength;

	if (newLength > maximumLength())
		newLastSector -= newLength - maximumLength();

	if (newLastSector != partition().lastSector() && (partition().children().size() == 0 || checkAlignment(*partition().children().last(), partition().lastSector() - newLastSector)))
	{
		const qint64 originalLast = partition().lastSector();

		partition().setLastSector(newLastSector);
		partition().fileSystem().setLastSector(newLastSector);

		if (align())
			device().partitionTable()->alignPartition(device(), partition());

		if (partition().lastSector() != originalLast)
		{
			resizeLogicals();
			updatePositions();
			emit lastSectorChanged(partition().lastSector());
			return true;
		}
	}

	return false;
}

/** Updates the Partition's length
	@param newLength the new length
	@return true on success
*/
bool PartResizerWidget::updateLength(qint64 newLength)
{
	newLength = qBound(minimumLength(), newLength, qMin(totalSectors(), maximumLength()));

	if (newLength == partition().length())
		return false;

	const qint64 oldLength = partition().length();
	qint64 delta = newLength - oldLength;

	qint64 tmp = qMin(delta, maximumLastSector() - partition().lastSector());
	delta -= tmp;

	if (tmp != 0)
	{
		partition().setLastSector(partition().lastSector() + tmp);
		partition().fileSystem().setLastSector(partition().lastSector() + tmp);

		emit lastSectorChanged(partition().lastSector());
	}

	tmp = qMin(delta, partition().firstSector() - minimumFirstSector());
	delta -= tmp;

	if (tmp != 0)
	{
		partition().setFirstSector(partition().firstSector() - tmp);
		partition().fileSystem().setFirstSector(partition().firstSector() - tmp);

		emit firstSectorChanged(partition().firstSector());
	}

	if (partition().length() != oldLength)
	{
		emit lengthChanged(partition().length());
		updatePositions();

		return true;
	}

	return false;
}

/** Sets the minimum sectors the Partition can be long.
	@note This value can never be less than 0 and never be higher than totalSectors()
	@param s the new minimum length
*/
void PartResizerWidget::setMinimumLength(qint64 s)
{
	m_MinimumLength = qBound(0LL, s, totalSectors());
}

/** Sets the maximum sectors the Partition can be long.
	@note This value can never be less than 0 and never by higher than totalSectors()
	@param s the new maximum length
*/
void PartResizerWidget::setMaximumLength(qint64 s)
{
	m_MaximumLength = qBound(0LL, s, totalSectors());
}

/** Sets if moving the Partition is allowed.
	@param b true if moving is allowed
*/
void PartResizerWidget::setMoveAllowed(bool b)
{
	m_MoveAllowed = b;

	if (!b)
		partWidget().setCursor(Qt::ArrowCursor);
}
