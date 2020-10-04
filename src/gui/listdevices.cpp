/*
    SPDX-FileCopyrightText: 2009-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/listdevices.h"

#include <core/device.h>

#include <util/globallog.h>
#include <util/capacity.h>

class ListDeviceWidgetItem : public QListWidgetItem
{
public:
    ListDeviceWidgetItem(const Device& d) :
        QListWidgetItem(QIcon::fromTheme(d.iconName()), d.prettyName()), deviceNode(d.deviceNode()) {
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
    m_ActionCollection(nullptr)
{
    setupUi(this);
}

void ListDevices::updateDevices(const OperationStack::Devices& devices)
{
    listDevices().clear();

    for (const auto &d : devices)
        listDevices().addItem(new ListDeviceWidgetItem(*d));
}

void ListDevices::on_m_ListDevices_itemSelectionChanged()
{
    if (listDevices().selectedItems().size() == 1) {
        ListDeviceWidgetItem* item = dynamic_cast<ListDeviceWidgetItem*>(listDevices().selectedItems()[0]);

        if (item != nullptr)
            Q_EMIT selectionChanged(item->deviceNode);
    }
}

void ListDevices::on_m_ListDevices_itemDoubleClicked(QListWidgetItem* list_item)
{
    ListDeviceWidgetItem* item = dynamic_cast<ListDeviceWidgetItem*>(list_item);

    if (item != nullptr)
        Q_EMIT deviceDoubleClicked(item->deviceNode);
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
