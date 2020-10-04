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
    CreateVolumeGroupDialog(QWidget* parent, QString& vgName, QVector<const Partition*>& pvList, QString& type,
                            qint32& raidLevel, qint32& chunkSize, qint32& peSize, QList<Device*> devices,
                            QList<Operation*> pendingOps = QList<Operation *>());

protected:
    void accept() override;
    void setupDialog() override;
    void setupConnections() override;
    void updatePartitionList() override;

protected:
    virtual void updateOkButtonStatus() override;
    void onVGNameChanged(const QString& vgname);
    void onSpinPESizeChanged(int newsize);

    QString& type() const {
        return m_type;
    }

    qint32& peSize() const {
        return m_PESize;
    }

    qint32& raidLevel() const {
        return m_raidLevel;
    }

    qint32& chunkSize() const {
        return m_chunkSize;
    }

    QString& m_type;

    qint32& m_PESize;
    qint32& m_raidLevel;
    qint32& m_chunkSize;

private:
    const QList<Device*> m_Devices; // List of all devices found on the system
    const QList<Operation*> m_PendingOps; // List of pending operations in KPM
};

#endif
