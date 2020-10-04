/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/listphysicalvolumes.h"

#include <fs/lvm2_pv.h>
#include <util/globallog.h>
#include <util/capacity.h>

ListPhysicalVolumes::ListPhysicalVolumes(QWidget* parent) :
    QWidget(parent),
    Ui::ListPhysicalVolumesBase()
{
    setupUi(this);
}

void ListPhysicalVolumes::addPartition(const Partition& p, bool checked)
{
    ListPhysicalVolumeWidgetItem *item = new ListPhysicalVolumeWidgetItem(p, checked);
    listPhysicalVolumes().addItem(item);
}

void ListPhysicalVolumes::clear()
{
    listPhysicalVolumes().clear();
}

QVector<const Partition *> ListPhysicalVolumes::checkedItems()
{
    QVector<const Partition *> partitionList;
    for (int i = 0; i < listPhysicalVolumes().count(); i++) {
        ListPhysicalVolumeWidgetItem* item = dynamic_cast<ListPhysicalVolumeWidgetItem*>(listPhysicalVolumes().item(i));
        if(item && item->checkState() == Qt::Checked)
            partitionList.push_back(item->partition());
    }
    return partitionList;
}

ListPhysicalVolumeWidgetItem::ListPhysicalVolumeWidgetItem(const Partition& p, bool checked)
    : QListWidgetItem(xi18nc("@item:inlistbox Device | Capacity", "%1 | %2",  p.deviceNode(), Capacity::formatByteSize(p.capacity())))
    , m_Partition(&p)
{
    setToolTip(p.deviceNode());
    setSizeHint(QSize(0, 32));
    setCheckState( checked ? Qt::Checked : Qt::Unchecked);
}
