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

#include "gui/partresizerwidget.h"
#include "gui/partwidget.h"

#include "core/partition.h"
#include "core/device.h"

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
	m_SectorsBefore(0),
	m_SectorsAfter(0),
	m_TotalSectors(-1),
	m_MinimumSectors(-1),
	m_MaximumSectors(-1),
	m_MaxFirstSector(-1),
	m_MinLastSector(-1),
	m_LeftHandle(this),
	m_RightHandle(this),
	m_DraggedWidget(NULL),
	m_Hotspot(0),
	m_MoveAllowed(true),
	m_ReadOnly(false)
{
}

/** Intializes the PartResizerWidget
	@param d the Device the Partition is on
	@param p the Partition to show and/or resize
	@param freeBefore number of sectors free before the Partition
	@param freeAfter number of sectors free after the Partition
*/
void PartResizerWidget::init(Device& d, Partition& p, qint64 freeBefore, qint64 freeAfter)
{
	setDevice(d);
	setPartition(p);

	setSectorsBefore(freeBefore);
	setSectorsAfter(freeAfter);

	setTotalSectors(sectorsBefore() + partition().length() + sectorsAfter());

	setMinimumSectors(qMax(partition().sectorsUsed(), partition().minimumSectors()));
	setMaximumSectors(qMin(totalSectors(), partition().maximumSectors()));

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
	}

	updatePositions();
}

qint64 PartResizerWidget::sectorsPerPixel() const
{
	return totalSectors() / (width() - 2 * handleWidth());
}

int PartResizerWidget::partWidgetStart() const
{
	return handleWidth() + sectorsBefore() / sectorsPerPixel();
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

void PartResizerWidget::mouseMoveEvent(QMouseEvent* event)
{
	int x = event->pos().x() - m_Hotspot;

	if (draggedWidget() == &leftHandle())
	{
		const qint64 newSectorsBefore = qMax(x * sectorsPerPixel(), 0LL);
		updateSectorsBefore(newSectorsBefore);
	}
	else if (draggedWidget() == &rightHandle())
	{
		const qint64 newSectorsAfter = qMax((width() - rightHandle().width() - x) * sectorsPerPixel(), 0LL);
		updateSectorsAfter(newSectorsAfter);
	}
	else if (draggedWidget() == &partWidget())
	{
		x -= handleWidth();
		qint64 newSectorsBefore = qMax(x * sectorsPerPixel(), 0LL);
		qint64 newSectorsAfter = sectorsAfter() + sectorsBefore() - newSectorsBefore;

		if (newSectorsAfter < 0)
		{
			newSectorsAfter = 0;
  			newSectorsBefore = sectorsBefore() + sectorsAfter();
		}

		if (newSectorsBefore != sectorsBefore() && newSectorsAfter != sectorsAfter())
			updateSectors(newSectorsBefore, newSectorsAfter);
	}
}

/** Updates the start and end sector of the Partition.
	@param newSectorsBefore new value for free sectors before the Partition
	@param newSectorsAfter new value for free sectors after the Partition
	@return true on success
*/
bool PartResizerWidget::updateSectors(qint64 newSectorsBefore, qint64 newSectorsAfter)
{
	Q_ASSERT(newSectorsBefore >= 0);
	Q_ASSERT(newSectorsAfter >= 0);
	Q_ASSERT(newSectorsBefore + newSectorsAfter + partition().length() == totalSectors());

	if (newSectorsBefore < 0 || newSectorsAfter < 0)
	{
		kWarning() << "new sectors before partition: " << newSectorsBefore;
		kWarning() << "new sectors after partition: " << newSectorsBefore;
		return false;
	}

	if (newSectorsBefore + newSectorsAfter + partition().length() != totalSectors())
	{
		kWarning() << "total sectors: " << totalSectors();
		kWarning() << "new sectors before partition: " << newSectorsBefore;
		kWarning() << "new sectors after partition: " << newSectorsBefore;
		kWarning() << "partition length: " << partition().length();
		return false;
	}
	
	if (!moveAllowed())
		return false;

	const qint64 oldBefore = sectorsBefore();
	const qint64 oldAfter = sectorsAfter();

	// Two hacky things about updating free sectors before and after a partition in one go:
	// 1) We have to call updateSectorsBefore and updateSectorsAfter with length-checking disabled,
	//    because the partition might in between those calls get smaller or bigger than
	//    allowed. The second call will, of course, restore the original length.
	// 2) If the user moves the mouse fast enough, it's possible to move the beginning past the
	//    end or the end in front of the beginning of the partition in between the calls. Both
	//    methods won't allow that and return false in that case. We try moving the beginning first and
	//    just move the end first if that fails.
	if (!updateSectorsBefore(newSectorsBefore, false))
	{
		updateSectorsAfter(newSectorsAfter, false);
		updateSectorsBefore(newSectorsBefore, false);
	}
	else
		updateSectorsAfter(newSectorsAfter, false);

	bool rval = false;
	
	if (oldBefore != sectorsBefore())
	{
		rval = true;
		emit sectorsBeforeChanged(sectorsBefore());
	}

	if (oldAfter != sectorsAfter())
	{
		rval = true;
		emit sectorsAfterChanged(sectorsAfter());
	}

	if (rval)
		updatePositions();

	return rval;
}

void PartResizerWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		m_DraggedWidget = NULL;
}

/** Updates the free sectors before the Partition
	@param newSectorsBefore new value for number of sectors free before Partition
	@param enableLengthCheck true if the method is supposed to do some sanity checking on the Partition length
	@return true on success
*/
bool PartResizerWidget::updateSectorsBefore(qint64 newSectorsBefore, bool enableLengthCheck)
{
	Q_ASSERT(newSectorsBefore >= 0);

	if (newSectorsBefore < 0)
	{
		kWarning() << "new sectors before partition: " << newSectorsBefore;
		return false;
	}
	
	const qint64 oldSectorsBefore = sectorsBefore();
	const qint64 newLength = partition().length() + oldSectorsBefore - newSectorsBefore;

	if (enableLengthCheck)
	{
		if (newLength < minimumSectors())
			newSectorsBefore -= minimumSectors() - newLength;

		if (newLength > maximumSectors())
			newSectorsBefore += newLength - maximumSectors();
	}
	else if (newLength < 0)
		return false;
	
	qint64 newFirstSector = partition().firstSector() + newSectorsBefore - oldSectorsBefore;

	if (maxFirstSector() > -1 && newFirstSector > maxFirstSector())
	{
		newSectorsBefore -= newFirstSector - maxFirstSector();
		newFirstSector = maxFirstSector();
	}

	if (newSectorsBefore >= 0 && newSectorsBefore != oldSectorsBefore && (partition().children().size() == 0 || checkSnap(*partition().children().first(), oldSectorsBefore - newSectorsBefore)))
	{
		setSectorsBefore(newSectorsBefore);

		partition().setFirstSector(newFirstSector);
		partition().fileSystem().setFirstSector(newFirstSector);
		
		resizeLogicals();

		emit sectorsBeforeChanged(sectorsBefore());
  		emit lengthChanged(partition().length());
	
		updatePositions();

		return true;
	}

	return false;
}

bool PartResizerWidget::checkSnap(const Partition& child, qint64 delta) const
{
	if (!partition().roles().has(PartitionRole::Extended))
		return true;

	if (child.roles().has(PartitionRole::Unallocated))
		return true;

	return qAbs(delta) >= Partition::minimumPartitionSectors();
}

void PartResizerWidget::resizeLogicals()
{
	if (!partition().roles().has(PartitionRole::Extended) || partition().children().size() == 0)
		return;

	device().partitionTable().removeUnallocated(&partition());
	device().partitionTable().insertUnallocated(device(), &partition(), partition().firstSector());
	
	partWidget().updateChildren();
}

/** Updates the number of free sectors after the Partition.
	@param newSectorsAfter new value for number of sectors free after Partition
	@param enableLengthCheck true if the method is supposed to do some sanity checking on the Partition length
	@return true on success
*/
bool PartResizerWidget::updateSectorsAfter(qint64 newSectorsAfter, bool enableLengthCheck)
{
	Q_ASSERT(newSectorsAfter >= 0);

	if (newSectorsAfter < 0)
	{
		kWarning() << "new sectors after partition: " << newSectorsAfter;
		return false;
	}

	const qint64 oldSectorsAfter = sectorsAfter();
	const qint64 newLength = partition().length() + oldSectorsAfter - newSectorsAfter;

	if (enableLengthCheck)
	{
		if (newLength < minimumSectors())
			newSectorsAfter -= minimumSectors() - newLength;

		if (newLength > maximumSectors())
			newSectorsAfter += newLength - maximumSectors();
	}
	else if (newLength < 0)
		return false;

	qint64 newLastSector = partition().lastSector() + oldSectorsAfter - newSectorsAfter;

	if (minLastSector() > -1 && newLastSector < minLastSector())
	{
		newSectorsAfter += newLastSector - minLastSector();
		newLastSector = minLastSector();
	}

	if (newSectorsAfter >= 0 && newSectorsAfter != oldSectorsAfter && (partition().children().size() == 0 || checkSnap(*partition().children().last(), oldSectorsAfter - newSectorsAfter)))
	{
		setSectorsAfter(newSectorsAfter);

		partition().setLastSector(newLastSector);
		partition().fileSystem().setLastSector(newLastSector);
		
		resizeLogicals();

 		emit sectorsAfterChanged(sectorsAfter());
		emit lengthChanged(partition().length());
	
		updatePositions();

		return true;
	}

	return false;
}

/** Updates the Partition's length
	@param newLength the new length
	@return true on success
*/
bool PartResizerWidget::updateLength(qint64 newLength)
{
	newLength = qBound(minimumSectors(), newLength, qMin(totalSectors(), maximumSectors()));

	if (newLength == partition().length())
		return false;

	const qint64 oldLength = partition().length();
	qint64 delta = newLength - oldLength;

	qint64 tmp = qMin(delta, sectorsAfter());
	delta -= tmp;

	if (tmp != 0)
	{
		setSectorsAfter(sectorsAfter() - tmp);
		partition().setLastSector(partition().lastSector() + tmp);
		partition().fileSystem().setLastSector(partition().lastSector() + tmp);
		emit sectorsAfterChanged(sectorsAfter());
	}

	tmp = qMin(delta, sectorsBefore());;
	delta -= tmp;

	if (tmp != 0)
	{
		setSectorsBefore(sectorsBefore() - tmp);
		partition().setFirstSector(partition().firstSector() - tmp);
		partition().fileSystem().setFirstSector(partition().firstSector() - tmp);
		emit sectorsBeforeChanged(sectorsBefore());
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
void PartResizerWidget::setMinimumSectors(qint64 s)
{
	m_MinimumSectors = qBound(0LL, s, totalSectors());
}

/** Sets the maximum sectors the Partition can be long.
	@note This value can never be less than 0 and never by higher than totalSectors()
	@param s the new maximum length
*/
void PartResizerWidget::setMaximumSectors(qint64 s)
{
	m_MaximumSectors = qBound(0LL, s, totalSectors());
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
