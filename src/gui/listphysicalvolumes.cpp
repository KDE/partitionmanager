/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
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

#include <core/lvmdevice.h>

#include <util/globallog.h>
#include <util/capacity.h>

#include <KIconLoader>

class ListPhysicalVolumeWidgetItem : public QListWidgetItem
{
public:
    ListPhysicalVolumeWidgetItem(const QString& pvnode) :
        QListWidgetItem(pvnode)
    {
        setToolTip(pvnode);
        setSizeHint(QSize(0, 32));
    }
};

ListPhysicalVolumes::ListPhysicalVolumes(QWidget* parent) :
    QWidget(parent),
    Ui::ListPhysicalVolumesBase()
{
    setupUi(this);
    listPhysicalVolumes().addItem(new ListPhysicalVolumeWidgetItem(QStringLiteral("TESTING STRING")));
}

void selectionToggled(const QString& pvnode)
{
    Q_UNUSED(pvnode);
}

