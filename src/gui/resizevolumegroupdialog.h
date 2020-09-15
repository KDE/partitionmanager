/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef RESIZEVOLUMEGROUPDIALOG_H
#define RESIZEVOLUMEGROUPDIALOG_H

#include <fs/lvm2_pv.h>

#include "gui/volumegroupdialog.h"

class Device;
class Operation;
class VolumeManagerDevice;

class ResizeVolumeGroupDialog : public VolumeGroupDialog
{
    Q_DISABLE_COPY(ResizeVolumeGroupDialog)

public:
    ResizeVolumeGroupDialog(QWidget* parent, VolumeManagerDevice *d, QVector<const Partition*>& partList, QList<Device*> devices, QList<Operation*> pendingOps = QList<Operation *>());

protected:
    void accept() override;
    void setupDialog() override;
    void setupConstraints() override;

    VolumeManagerDevice* device() const {
        return m_Device;
    }

private:
    const QList<Device*> m_Devices; // List of all devices found on the system
    VolumeManagerDevice* m_Device;
    const QList<Operation*> m_PendingOps; // List of pending operations in KPM
};

#endif
