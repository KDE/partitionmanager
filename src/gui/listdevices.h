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

#if !defined(LISTDEVICES__H)

#define LISTDEVICES__H

#include <core/operationstack.h>

#include "ui_listdevicesbase.h"

#include <QWidget>

class Device;
class QPoint;
class KActionCollection;

/** A list of devices.
    @author Volker Lanz <vl@fidra.de>
*/
class ListDevices : public QWidget, public Ui::ListDevicesBase
{
    Q_OBJECT
    Q_DISABLE_COPY(ListDevices)

public:
    ListDevices(QWidget* parent = nullptr);

Q_SIGNALS:
    void selectionChanged(const QString& device_node);
    void deviceDoubleClicked(const QString& device_node);

public:
    void setActionCollection(KActionCollection* coll) {
        m_ActionCollection = coll;
    }
    bool setSelectedDevice(const QString& device_node);

    void updateDevices(const OperationStack::Devices& devices);

    QListWidget& listDevices() {
        Q_ASSERT(m_ListDevices);
        return *m_ListDevices;
    }
protected:
    const QListWidget& listDevices() const {
        Q_ASSERT(m_ListDevices);
        return *m_ListDevices;
    }
    KActionCollection* actionCollection() {
        return m_ActionCollection;
    }

    void customContextMenuRequested(const QPoint& pos);

protected Q_SLOTS:
    void on_m_ListDevices_itemSelectionChanged();
    void on_m_ListDevices_itemDoubleClicked(QListWidgetItem* list_item);

private:
    KActionCollection* m_ActionCollection;
};

#endif

