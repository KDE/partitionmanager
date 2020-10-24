/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/createvolumegroupdialog.h"
#include "gui/volumegroupwidget.h"

#include <core/device.h>
#include <core/lvmdevice.h>
#include <core/partitiontable.h>

#include <fs/lvm2_pv.h>

#include <ops/deleteoperation.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <utility>

#include <QtGlobal>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

CreateVolumeGroupDialog::CreateVolumeGroupDialog(QWidget* parent, QString& vgName, QVector<const Partition*>& partList, qint32& peSize, QList<Device*> devices, QList<Operation*> pendingOps)
    : VolumeGroupDialog(parent, vgName, partList)
    , m_PESize(peSize)
    , m_Devices(devices)
    , m_PendingOps(pendingOps)
{
    setWindowTitle(xi18nc("@title:window", "Create new Volume Group"));

    setupDialog();
    setupConstraints();
    setupConnections();

    // disable volume type and PE size for now, until the features are implemented.
    dialogWidget().volumeType().setEnabled(false);

    KConfigGroup kcg(KSharedConfig::openConfig(), "createVolumeDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

void CreateVolumeGroupDialog::setupDialog()
{
    for (const auto &p : std::as_const(LVM::pvList::list())) {
        bool toBeDeleted = false;

        // Ignore partitions that are going to be deleted
        for (const auto &o : std::as_const(m_PendingOps)) {
            if (dynamic_cast<DeleteOperation *>(o) && o->targets(*p.partition())) {
                toBeDeleted = true;
                break;
            }
        }

        if (toBeDeleted)
            continue;

        if (!p.isLuks() && p.vgName().isEmpty() && !LvmDevice::s_DirtyPVs.contains(p.partition()))
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
}

void CreateVolumeGroupDialog::setupConnections()
{
    connect(&dialogWidget().vgName(), &QLineEdit::textChanged, this, &CreateVolumeGroupDialog::onVGNameChanged);
    connect(&dialogWidget().spinPESize(), qOverload<int>(&QSpinBox::valueChanged), this, &CreateVolumeGroupDialog::onSpinPESizeChanged);
}

void  CreateVolumeGroupDialog::accept()
{
    QString& tname = targetName();
    tname = dialogWidget().vgName().text();

    targetPVList().append(dialogWidget().listPV().checkedItems());

    qint32& pesize = peSize();
    pesize = dialogWidget().spinPESize().value();

    QDialog::accept();
}

void CreateVolumeGroupDialog::updateOkButtonStatus()
{
    VolumeGroupDialog::updateOkButtonStatus();

    if (okButton->isEnabled())
        okButton->setEnabled(!dialogWidget().listPV().checkedItems().empty());
}

void CreateVolumeGroupDialog::onVGNameChanged(const QString& vgName)
{
    for (const auto &d : m_Devices) {
        if (dynamic_cast<LvmDevice*>(d)) {
            if (d->name() == vgName) {
                m_IsValidName = false;
                break;
            }
            else
                m_IsValidName = true;
        }
    }
    updateOkButtonStatus();
}

void CreateVolumeGroupDialog::onSpinPESizeChanged(int newsize)
{
    Q_UNUSED(newsize);
    updateOkButtonStatus();
    updateSectorInfos();
}
