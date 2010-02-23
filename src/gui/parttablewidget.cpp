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

#include "gui/parttablewidget.h"
#include "gui/partwidget.h"

#include "core/partitiontable.h"

#include <QMouseEvent>

#include <kdebug.h>
#include <klocale.h>

/** Creates a new PartTableWidget.
	@param parent pointer to the parent widget
*/
PartTableWidget::PartTableWidget(QWidget* parent) :
	QWidget(parent),
	m_PartitionTable(NULL),
	m_Widgets(),
	m_ActiveWidget(NULL),
	m_LabelEmpty(i18nc("@info", "Please select a device."), this),
	m_ReadOnly(false)
{
	labelEmpty().setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
}

/** Sets the PartitionTable this widget shows.
	@param ptable pointer to the PartitionTable to show. Must not be NULL.
*/
void PartTableWidget::setPartitionTable(const PartitionTable* ptable)
{
	clear();

	m_PartitionTable = ptable;

	if (partitionTable() != NULL)
	{
		foreach(const Partition* p, partitionTable()->children())
		{
			widgets().append(new PartWidget(this, this, p));
			widgets().last()->show();
		}
	}

	if (widgets().isEmpty())
	{
		labelEmpty().setVisible(true);
		labelEmpty().setText(i18nc("@info", "No valid partition table was found on this device."));
		labelEmpty().resize(size());
	}
	else
	{
		labelEmpty().setVisible(false);
		positionChildren(this, partitionTable()->children(), widgets());
	}

	update();
}

/** Sets a widget active.
	@param p pointer to the PartWidget to set active. May be NULL.
*/
void PartTableWidget::setActiveWidget(PartWidget* p)
{
	if (isReadOnly())
		return;

	const PartWidget* old = m_ActiveWidget;

	m_ActiveWidget = p;

	if (old != activeWidget())
		emit itemSelectionChanged(p);

	update();
}

/** Sets a widget for the given Partition active.
	@param p pointer to the Partition whose widget is to be set active. May be NULL.
*/
void PartTableWidget::setActivePartition(const Partition* p)
{
	if (isReadOnly())
		return;

	foreach (PartWidget* pw, findChildren<PartWidget*>())
		if (pw->partition() == p)
		{
			setActiveWidget(pw);
			return;
		}

	setActiveWidget(NULL);
}

/** Clears the PartTableWidget.
*/
void PartTableWidget::clear()
{
	setActiveWidget(NULL);
	m_PartitionTable = NULL;

	// we might have been invoked indirectly via a widget's context menu, so
	// that its event handler is currently running. therefore, do not delete
	// the part widgets here but schedule them for deletion once the app
	// returns to the main loop (and the event handler has finished).
	foreach(PartWidget* p, widgets())
	{
		p->setVisible(false);
		p->deleteLater();
	}

	widgets().clear();

	update();
}

void PartTableWidget::resizeEvent(QResizeEvent*)
{
	if (partitionTable() == NULL || widgets().isEmpty())
		labelEmpty().resize(size());
	else
		positionChildren(this, partitionTable()->children(), widgets());
}

void PartTableWidget::mousePressEvent(QMouseEvent* event)
{
	if (isReadOnly())
		return;

	event->accept();
	PartWidget* child = qobject_cast<PartWidget*>(childAt(event->pos()));
	setActiveWidget(child);
}

void PartTableWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (isReadOnly() || event->button() != Qt::LeftButton)
		return;

	event->accept();

	const PartWidget* child = static_cast<PartWidget*>(childAt(event->pos()));

	if (child != NULL)
		emit itemDoubleClicked(child);
}
