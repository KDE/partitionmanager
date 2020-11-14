/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/resizevolumegroupdialog.h"
#include "gui/volumegroupwidget.h"

#include <core/lvmdevice.h>
#include <core/volumemanagerdevice.h>
#include <core/partitiontable.h>
#include <core/softwareraid.h>

#include <fs/lvm2_pv.h>

#include <ops/deleteoperation.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <utility>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QDebug>

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
<<<<<<< HEAD
    if (device()->type() == Device::Type::LVM_Device) {
        dialogWidget().volumeType().setCurrentIndex(0);

        for (const auto &p : qAsConst(LVM::pvList::list())) {
=======
    if (dialogWidget().volumeType().currentText() == QStringLiteral("LVM")) {
        for (const auto &p : std::as_const(LVM::pvList::list())) {
>>>>>>> master
            bool toBeDeleted = false;

            // Ignore partitions that are going to be deleted
            for (const auto &o : std::as_const(m_PendingOps)) {
                if (dynamic_cast<DeleteOperation *>(o) && o->targets(*p.partition())) {
                    toBeDeleted = true;
                    break;
                }
            }

            if (toBeDeleted || p.isLuks())
                continue;

            if (p.vgName() == device()->name())
                dialogWidget().listPV().addPartition(*p.partition(), true);
            else if (p.vgName().isEmpty() && !LvmDevice::s_DirtyPVs.contains(p.partition())) // TODO: Remove LVM PVs in current VG
                dialogWidget().listPV().addPartition(*p.partition(), false);
        }

        for (const Device *d : std::as_const(m_Devices)) {
            if (d->partitionTable() != nullptr) {
                for (const Partition *p : std::as_const(d->partitionTable()->children())) {
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

        for (const Partition *p : std::as_const(LvmDevice::s_OrphanPVs))
            if (!LvmDevice::s_DirtyPVs.contains(p))
                dialogWidget().listPV().addPartition(*p, false);

        LvmDevice *lvmDevice = static_cast<LvmDevice*>(device());

        m_TotalUsedSize = lvmDevice->allocatedPE() * lvmDevice->peSize();

        dialogWidget().totalUsedSize().setText(Capacity::formatByteSize(m_TotalUsedSize));
    }
    else if (device()->type() == Device::Type::SoftwareRAID_Device) {
        dialogWidget().volumeType().setCurrentIndex(1);

        for (const Device *d : qAsConst(m_Devices)) {
            if (d != device() && d->partitionTable() != nullptr) {
                for (const Partition *p : qAsConst(d->partitionTable()->children())) {
                    QString arrayName = SoftwareRAID::getRaidArrayName(p->partitionPath());
                    if (arrayName == device()->deviceNode())
                        dialogWidget().listPV().addPartition(*p, true);
                    else if (((p->fileSystem().type() == FileSystem::Type::LinuxRaidMember &&
                          arrayName.isEmpty()) || p->fileSystem().type() == FileSystem::Type::Unformatted ||
                          p->fileSystem().type() == FileSystem::Type::Unknown) && !p->roles().has(PartitionRole::Role::Unallocated))
                        dialogWidget().listPV().addPartition(*p, false);
                }
            }
        }

        SoftwareRAID* raid = static_cast<SoftwareRAID*>(device());

        m_TotalUsedSize = 0;

        for (const Partition* p : device()->partitionTable()->children())
            if (!p->roles().has(PartitionRole::Unallocated))
                m_TotalUsedSize += p->used();

        dialogWidget().totalUsedSize().setText(Capacity::formatByteSize(m_TotalUsedSize));

        int index = dialogWidget().raidLevel().findText(QString::number(raid->raidLevel()));

        if (index != -1)
            dialogWidget().raidLevel().setCurrentIndex(index);

        dialogWidget().chunkSize().setValue(raid->chunkSize());
    }

    int totalLV = 0;

    if (device()->partitionTable()) {
        for (const Partition* p : device()->partitionTable()->children())
            if (!p->roles().has(PartitionRole::Role::Unallocated))
                totalLV++;
    }

    dialogWidget().totalLV().setText(QString::number(totalLV));
}

void ResizeVolumeGroupDialog::setupConstraints()
{
    dialogWidget().vgName().setEnabled(false);
    dialogWidget().spinPESize().setEnabled(false);
    dialogWidget().volumeType().setEnabled(false);

    // set constraints for raid
    dialogWidget().raidLevel().setEnabled(false);
    dialogWidget().chunkSize().setEnabled(false);

    VolumeGroupDialog::setupConstraints();
}

void ResizeVolumeGroupDialog::accept()
{
    targetPVList().append(dialogWidget().listPV().checkedItems());
    QDialog::accept();
}
