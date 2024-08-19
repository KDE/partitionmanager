/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/


#ifndef CREATEVOLUMEGROUPDIALOG_H
#define CREATEVOLUMEGROUPDIALOG_H

#include <core/device.h>
#include <fs/lvm2_pv.h>

#include "gui/volumegroupdialog.h"

class Device;
class Operation;

class CreateVolumeGroupDialog : public VolumeGroupDialog
{
    Q_DISABLE_COPY(CreateVolumeGroupDialog)

public:
    CreateVolumeGroupDialog(QWidget* parent, QString& vgName, QVector<const Partition*>& pvList, qint32& peSize, QList<Device*> devices, QList<Operation*> pendingOps = QList<Operation *>());

protected:
    void accept() override;
    void setupDialog() override;
    void setupConnections() override;

protected:
    void updateOkButtonStatus() override;
    void onVGNameChanged(const QString& vgname);
    void onSpinPESizeChanged(int newsize);

    qint32& peSize() {
        return m_PESize;
    }

    qint32& m_PESize;

private:
    const QList<Device*> m_Devices; // List of all devices found on the system
    const QList<Operation*> m_PendingOps; // List of pending operations in KPM
};

#endif
