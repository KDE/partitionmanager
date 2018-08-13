/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2018 by Caio Carvalho <caiojcarvalho@gmail.com>        *
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
