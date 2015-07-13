/*************************************************************************
 *  Copyright (C) 2008-2010 by Volker Lanz <vl@fidra.de>                 *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#include "gui/listdevices.h"

#include <kpmcore/core/device.h>

#include <kpmcore/util/globallog.h>
#include <kpmcore/util/capacity.h>

#include <KIconLoader>

class ListDeviceWidgetItem : public QListWidgetItem
{
public:
    ListDeviceWidgetItem(const Device& d) :
        QListWidgetItem(QIcon::fromTheme(d.iconName()).pixmap(IconSize(KIconLoader::Desktop)), d.prettyName()),
        deviceNode(d.deviceNode()) {
        setToolTip(d.prettyName());
        setSizeHint(QSize(0, 32));
    }

    const QString deviceNode;
};

/** Creates a new ListDevices instance.
    @param parent the parent widget
*/
ListDevices::ListDevices(QWidget* parent) :
    QWidget(parent),
    Ui::ListDevicesBase(),
    m_ActionCollection(NULL)
{
    setupUi(this);
}

void ListDevices::updateDevices(OperationStack::Devices& devices)
{
    listDevices().clear();

    foreach(const Device * d, devices)
    listDevices().addItem(new ListDeviceWidgetItem(*d));
}

void ListDevices::on_m_ListDevices_itemSelectionChanged()
{
    if (listDevices().selectedItems().size() == 1) {
        ListDeviceWidgetItem* item = dynamic_cast<ListDeviceWidgetItem*>(listDevices().selectedItems()[0]);

        if (item != NULL)
            emit selectionChanged(item->deviceNode);
    }
}

void ListDevices::on_m_ListDevices_customContextMenuRequested(const QPoint& pos)
{
    emit contextMenuRequested(listDevices().viewport()->mapToGlobal(pos));
}

void ListDevices::on_m_ListDevices_itemDoubleClicked(QListWidgetItem* list_item)
{
    ListDeviceWidgetItem* item = dynamic_cast<ListDeviceWidgetItem*>(list_item);

    if (item != NULL)
        emit deviceDoubleClicked(item->deviceNode);
}

bool ListDevices::setSelectedDevice(const QString& device_node)
{
    QList<QListWidgetItem*> items = listDevices().findItems(device_node, Qt::MatchContains);

    if (items.size() > 0) {
        listDevices().setCurrentItem(items[0]);
        return true;
    }

    return false;
}
