/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "gui/resizevolumegroupdialog.h"
#include "gui/volumegroupwidget.h"

#include <core/lvmdevice.h>
#include <core/volumemanagerdevice.h>
#include <core/partitiontable.h>
#include <fs/lvm2_pv.h>

#include <ops/deleteoperation.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/** Creates a new ResizeVolumeGroupDialog
    @param parent pointer to the parent widget
    @param d the Device to show properties for
*/
ResizeVolumeGroupDialog::ResizeVolumeGroupDialog(QWidget* parent, VolumeManagerDevice* d, QVector<const Partition*>& partList, QList<Device*> devices, QList<Operation*> pendingOps)
    : VolumeGroupDialog(parent, d->name(), partList)
    , m_Devices(devices)
    , m_Device(d)
    , m_PendingOps(pendingOps)
{
    setWindowTitle(xi18nc("@title:window", "Resize Volume Group"));

    setupDialog();
    setupConstraints();

    KConfigGroup kcg(KSharedConfig::openConfig(), "resizeVolumeDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

void ResizeVolumeGroupDialog::setupDialog()
{
    if (dialogWidget().volumeType().currentText() == QStringLiteral("LVM")) {
        for (const auto &p : qAsConst(LVM::pvList::list())) {
            bool toBeDeleted = false;

            // Ignore partitions that are going to be deleted
            for (const auto &o : qAsConst(m_PendingOps)) {
                if (dynamic_cast<DeleteOperation *>(o) && o->targets(*p.partition())) {
                    toBeDeleted = true;
                    break;
                }
            }

            if (toBeDeleted)
                continue;

            if (p.isLuks())
                continue;
            if (p.vgName() == device()->name())
                dialogWidget().listPV().addPartition(*p.partition(), true);
            else if (p.vgName().isEmpty() && !LvmDevice::s_DirtyPVs.contains(p.partition())) // TODO: Remove LVM PVs in current VG
                dialogWidget().listPV().addPartition(*p.partition(), false);
        }

        for (const Device *d : qAsConst(m_Devices)) {
            if (d->partitionTable() != nullptr) {
                for (const Partition *p : qAsConst(d->partitionTable()->children())) {
                    // Looking if there is another VG creation that contains this partition
                    if (LvmDevice::s_DirtyPVs.contains(p))
                        continue;

                    // Including new LVM PVs (that are currently in OperationStack and that aren't at other VG creation)
                    if (p->state() == Partition::State::New) {
                        if (p->fileSystem().type() == FileSystem::Type::Lvm2_PV)
                            dialogWidget().listPV().addPartition(*p, false);
                        else if (p->fileSystem().type() == FileSystem::Type::Luks || p->fileSystem().type() == FileSystem::Type::Luks2) {
                            FileSystem *fs = static_cast<const FS::luks *>(&p->fileSystem())->innerFS();

                            if (fs->type() == FileSystem::Type::Lvm2_PV)
                                dialogWidget().listPV().addPartition(*p, false);
                        }
                    }
                }
            }
        }

        for (const Partition *p : qAsConst(LvmDevice::s_OrphanPVs))
            if (!LvmDevice::s_DirtyPVs.contains(p))
                dialogWidget().listPV().addPartition(*p, false);
    }

    //update used size and LV infos
    qint32 totalLV = 0;
    LvmDevice *lvmDevice = dynamic_cast<LvmDevice *>(device());
    if (lvmDevice != nullptr) {
        m_TotalUsedSize = lvmDevice->allocatedPE() * lvmDevice->peSize();
        totalLV = lvmDevice->partitionTable()->children().count();
    }

    dialogWidget().totalUsedSize().setText(Capacity::formatByteSize(m_TotalUsedSize));
    dialogWidget().totalLV().setText(QString::number(totalLV));
}

void ResizeVolumeGroupDialog::setupConstraints()
{
    dialogWidget().vgName().setEnabled(false);
    dialogWidget().spinPESize().setEnabled(false);
    dialogWidget().volumeType().setEnabled(false);
    VolumeGroupDialog::setupConstraints();
}

void ResizeVolumeGroupDialog::accept()
{
    targetPVList().append(dialogWidget().listPV().checkedItems());
    QDialog::accept();
}
