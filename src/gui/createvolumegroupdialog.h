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

#if !defined(CREATEVOLUMEGROUPDIALOG__H)

#define CREATEVOLUMEGROUPDIALOG__H

#include <fs/lvm2_pv.h>

#include "gui/volumegroupdialog.h"

class Device;

class CreateVolumeGroupDialog : public VolumeGroupDialog
{
    Q_DISABLE_COPY(CreateVolumeGroupDialog)

public:
    CreateVolumeGroupDialog(QWidget* parent, FS::lvm2_pv::PhysicalVolumes physicalVolumes, QString& vgName, QStringList& pvList, qint32& peSize);

protected:
    void accept() override;
    void setupDialog() override;
    void setupConnections() override;

protected:
    void onVGNameChanged(const QString& vgname);
    void onSpinPESizeChanged(int newsize);

    qint32& peSize() {
        return m_PESize;
    }

    qint32& m_PESize;

private:
    QStringList m_SystemVGList;
    const FS::lvm2_pv::PhysicalVolumes m_PhysicalVolumes; // List of all devices found on the system
};

#endif
