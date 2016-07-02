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

#if !defined(LISTPHYSICALVOLUMES__H)

#define LISTPHYSICALVOLUMES__H

#include "ui_listphysicalvolumesbase.h"

#include <QWidget>

class Device;
class QPoint;
class KActionCollection;

class ListPhysicalVolumes : public QWidget, public Ui::ListPhysicalVolumesBase
{
    Q_OBJECT
    Q_DISABLE_COPY(ListPhysicalVolumes)

public:
    ListPhysicalVolumes(QWidget* parent = nullptr);

    void selectionToggled(const QString& pvnode);

protected:
    QListWidget& listPhysicalVolumes() {
        Q_ASSERT(m_ListPhysicalVolumes);
        return *m_ListPhysicalVolumes;
    }
    const QListWidget& listPhysicalVolumes() const {
        Q_ASSERT(m_ListPhysicalVolumes);
        return *m_ListPhysicalVolumes;
    }
};

#endif

