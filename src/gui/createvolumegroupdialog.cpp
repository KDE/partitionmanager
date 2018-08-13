/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "gui/createvolumegroupdialog.h"
#include "gui/volumegroupwidget.h"

#include <core/device.h>
#include <core/lvmdevice.h>
#include <core/partitiontable.h>

#include <fs/lvm2_pv.h>

#include <ops/deleteoperation.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <QtGlobal>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

CreateVolumeGroupDialog::CreateVolumeGroupDialog(QWidget* parent, QString& vgName, QVector<const Partition*>& partList,
                                                 QString& type, qint32& raidLevel, qint32& chunkSize, qint32& peSize,
                                                 QList<Device*> devices, QList<Operation*> pendingOps)
    : VolumeGroupDialog(parent, vgName, partList)
    , m_type(type)
    , m_PESize(peSize)
    , m_raidLevel(raidLevel)
    , m_chunkSize(chunkSize)
    , m_Devices(devices)
    , m_PendingOps(pendingOps)
{
    setWindowTitle(xi18nc("@title:window", "Create new Volume Group"));

    setupDialog();
    setupConstraints();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), "createVolumeDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

void CreateVolumeGroupDialog::setupDialog()
{
    updatePartitionList();
}

void CreateVolumeGroupDialog::setupConnections()
{
    connect(&dialogWidget().vgName(), &QLineEdit::textChanged, this, &CreateVolumeGroupDialog::onVGNameChanged);
    connect(&dialogWidget().spinPESize(), qOverload<int>(&QSpinBox::valueChanged), this, &CreateVolumeGroupDialog::onSpinPESizeChanged);
}

void CreateVolumeGroupDialog::updatePartitionList()
{
    if (dialogWidget().volumeType().currentText() == QStringLiteral("LVM")) {
        dialogWidget().listPV().clear();

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

            if (!p.isLuks() && p.vgName() == QString() && !LvmDevice::s_DirtyPVs.contains(p.partition()))
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
    else if (dialogWidget().volumeType().currentText() == QStringLiteral("RAID")) {
        for (const Device *d : qAsConst(m_Devices)) {
            if (d->type() != Device::Type::SoftwareRAID_Device && d->partitionTable() != nullptr) {
                for (const Partition *p : qAsConst(d->partitionTable()->children())) {
                    if ((p->fileSystem().type() == FileSystem::Type::LinuxRaidMember ||
                            p->fileSystem().type() == FileSystem::Type::Unformatted ||
                            p->fileSystem().type() == FileSystem::Type::Unknown) &&
                            !p->roles().has(PartitionRole::Role::Unallocated))
                        dialogWidget().listPV().addPartition(*p, false);
                }
            }
        }
    }
}

void CreateVolumeGroupDialog::accept()
{
    QString& tname = targetName();
    tname = dialogWidget().vgName().text();

    targetPVList().append(dialogWidget().listPV().checkedItems());

    QString& vgType = type();
    vgType = dialogWidget().volumeType().currentText();

    qint32& pesize = peSize();
    pesize = dialogWidget().spinPESize().value();

    qint32& raidlvl = raidLevel();
    raidlvl = dialogWidget().raidLevel().currentText().toInt();

    qint32& chunk = chunkSize();
    chunk = dialogWidget().chunkSize().value();

    QDialog::accept();
}

void CreateVolumeGroupDialog::updateOkButtonStatus()
{
    VolumeGroupDialog::updateOkButtonStatus();

    if (okButton->isEnabled()) {
        okButton->setEnabled(!dialogWidget().listPV().checkedItems().empty());

        if (dialogWidget().volumeType().currentText() == QStringLiteral("RAID")) {
            bool ok = dialogWidget().listPV().checkedItems().count() >= 2 &&
                    dialogWidget().vgName().text().startsWith(QStringLiteral("md"));

            okButton->setEnabled(ok);
        }
    }
}

void CreateVolumeGroupDialog::onVGNameChanged(const QString& vgName)
{
    for (const auto &d : m_Devices) {
        if (d->name() == vgName) {
            m_IsValidName = false;
            break;
        }
        else
            m_IsValidName = true;
    }
    updateOkButtonStatus();
}

void CreateVolumeGroupDialog::onSpinPESizeChanged(int newsize)
{
    Q_UNUSED(newsize);
    updateOkButtonStatus();
    updateSectorInfos();
}
