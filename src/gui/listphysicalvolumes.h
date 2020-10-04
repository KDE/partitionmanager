/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(LISTPHYSICALVOLUMES_H)

#define LISTPHYSICALVOLUMES_H

#include "ui_listphysicalvolumesbase.h"

#include <core/partition.h>

#include <QVector>

#include <QWidget>

class Device;
class QPoint;
class KActionCollection;

class ListPhysicalVolumeWidgetItem : public QListWidgetItem
{
public:
    ListPhysicalVolumeWidgetItem(const Partition& p, bool checked);
    const Partition* partition() const { return m_Partition; }

private:
    const Partition* m_Partition;
};

class ListPhysicalVolumes : public QWidget, public Ui::ListPhysicalVolumesBase
{
    Q_DISABLE_COPY(ListPhysicalVolumes)

public:
    explicit ListPhysicalVolumes(QWidget* parent = nullptr);

    void addPartition(const Partition& p, bool checked);

    void clear();

    QVector<const Partition *> checkedItems();

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

