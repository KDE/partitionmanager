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

#include "gui/listdevices.h"

#include "gui/partitionmanagerwidget.h"

#include "core/device.h"

#include "util/globallog.h"
#include "util/capacity.h"

#include <kmenu.h>
#include <kactioncollection.h>

/** Creates a new ListDevices instance.
	@param parent the parent widget
*/
ListDevices::ListDevices(QWidget* parent) :
	QWidget(parent),
	Ui::ListDevicesBase(),
	m_ActionCollection(NULL),
	m_PartitionManagerWidget(NULL)
{
	setupUi(this);
}

void ListDevices::updateDevices()
{
	listDevices().clear();

	foreach(const Device* d, pmWidget().previewDevices())
	{
		const QString shortText = d->deviceNode() + " (" + Capacity(*d).toString() + ')';
		const QString longText = d->deviceNode() + " (" + Capacity(*d).toString() + ", " + d->name() + ')';
		QListWidgetItem* item = new QListWidgetItem(SmallIcon("drive-harddisk"), shortText);
		item->setToolTip(longText);
		listDevices().addItem(item);
	}
}

void ListDevices::on_m_ListDevices_itemSelectionChanged()
{
	int idx = -1;

	if (listDevices().selectedItems().size() == 1)
		idx = listDevices().row(listDevices().selectedItems()[0]);

	Device* d = NULL;
	if (idx >= 0 && idx < pmWidget().previewDevices().size())
		d = pmWidget().previewDevices()[idx];

	emit selectionChanged(d);
}

void ListDevices::on_m_ListDevices_customContextMenuRequested(const QPoint& pos)
{
	Q_ASSERT(actionCollection());

	KMenu deviceMenu;
	deviceMenu.addAction(actionCollection()->action("createNewPartitionTable"));
	deviceMenu.exec(listDevices().viewport()->mapToGlobal(pos));
}
