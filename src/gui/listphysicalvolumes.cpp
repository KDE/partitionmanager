/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

QList<const Partition *> ListPhysicalVolumes::checkedItems()
{
    QList<const Partition *> partitionList;
    for (int i = 0; i < listPhysicalVolumes().count(); i++) {
        ListPhysicalVolumeWidgetItem* item = dynamic_cast<ListPhysicalVolumeWidgetItem*>(listPhysicalVolumes().item(i));
        if(item && item->checkState() == Qt::Checked)
            partitionList.append(item->partition());
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
