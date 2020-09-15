/*
    SPDX-FileCopyrightText: 2009-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(LISTDEVICES_H)

#define LISTDEVICES_H

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
    explicit ListDevices(QWidget* parent = nullptr);

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

