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

#include "gui/partwidget.h"
#include "gui/parttablewidget.h"

#include "util/capacity.h"

#include "core/partition.h"
#include "core/operationstack.h"

#include "fs/filesystem.h"

#include <QPainter>

#include <kdebug.h>
#include <kglobalsettings.h>

#include <config.h>

/** Creates a new PartWidget
	@param parent pointer to the parent widget
	@param ptWidget pointer to the PartTableWidget this widget will be in
	@param p pointer to the Partition this widget will show
	@param show_children true if this widget is supposed to show child widgets
*/
PartWidget::PartWidget(QWidget* parent, const PartTableWidget* ptWidget, const Partition* p, bool show_children) :
	QWidget(parent),
	PartWidgetBase(),
	m_PartTableWidget(const_cast<PartTableWidget*>(ptWidget)),
	m_Partition(const_cast<Partition*>(p)),
	m_Widgets(),
	m_ShowChildren(show_children)
{
	setFont(KGlobalSettings::smallestReadableFont());

	if (partition())
		setToolTip(partition()->deviceNode() + '\n' + partition()->fileSystem().name() + ' ' + Capacity(*partition()).toString());

	updateChildren();
}

/** Updates the widget's children */
void PartWidget::updateChildren()
{
	foreach (QWidget* w, widgets())
	{
		w->setVisible(false);
		w->deleteLater();
	}

	widgets().clear();

	if (partition() && showChildren())
	{
		foreach(Partition* child, partition()->children())
		{
			widgets().append(new PartWidget(this, partTableWidget(), child));
			widgets().last()->show();
		}

		positionChildren(this, partition()->children(), widgets());
	}
}

/** @return true if this is the currently active widget */
bool PartWidget::active() const
{
	if (partTableWidget() == NULL)
		return false;

	return partTableWidget()->activeWidget() == this;
}

void PartWidget::resizeEvent(QResizeEvent*)
{
 	if (partition() && showChildren())
 		positionChildren(this, partition()->children(), widgets());
}

void PartWidget::paintEvent(QPaintEvent*)
{
	drawPartition(this);
}

QColor PartWidget::activeColor(const QColor& col) const
{
	if (active())
		return col.darker(130);

	return col;
}

void PartWidget::drawPartition(QWidget* destWidget)
{
	if (partition() == NULL)
		return;

	const int usedPercentage = partition()->used() * 100 / partition()->capacity();
	const int w = (destWidget->width() - 1 - (PartWidget::borderWidth() * 2)) * usedPercentage / 100;

	QPainter painter(destWidget);

	// draw border
	painter.setPen(active() ? QColor(250, 250, 250) : QColor(20, 20, 20));
	painter.setBrush(activeColor(Config::fileSystemColorCode(partition()->fileSystem().type())));
	painter.drawRect(QRect(0, 0, destWidget->width() - 1, destWidget->height() - 1));

	if (partition()->roles().has(PartitionRole::Extended))
		return;

	if (!partition()->roles().has(PartitionRole::Unallocated))
	{
		// draw free space background
		painter.setBrush(activeColor(Config::availableSpaceColorCode()));
		painter.drawRect(QRect(PartWidget::borderWidth(), PartWidget::borderHeight(), destWidget->width() - 1 - (PartWidget::borderWidth() * 2), destWidget->height() - (PartWidget::borderHeight() * 2)));

		// draw used space in front of that
		painter.setBrush(activeColor(Config::usedSpaceColorCode()));
		painter.drawRect(QRect(PartWidget::borderWidth(), PartWidget::borderHeight(), w, destWidget->height() - (PartWidget::borderHeight() * 2)));
	}

	// draw name and size
	QString text = partition()->deviceNode().remove("/dev/") + '\n' + Capacity(*partition()).toString();

	const QRect textRect(0, 0, destWidget->width() - 1, destWidget->height() - 1);
	const QRect boundingRect = painter.boundingRect(textRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
	if (boundingRect.x() > PartWidgetBase::borderWidth() && boundingRect.y() > PartWidgetBase::borderHeight())
		painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
}

