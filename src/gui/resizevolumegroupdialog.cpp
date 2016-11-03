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

#include "gui/resizevolumegroupdialog.h"
#include "gui/volumegroupwidget.h"

#include <core/lvmdevice.h>
#include <core/volumemanagerdevice.h>
#include <core/partitiontable.h>
#include <fs/lvm2_pv.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/** Creates a new ResizeVolumeGroupDialog
    @param parent pointer to the parent widget
    @param d the Device to show properties for
*/
ResizeVolumeGroupDialog::ResizeVolumeGroupDialog(QWidget* parent, VolumeManagerDevice* d, QList<const Partition*>& partList)
    : VolumeGroupDialog(parent, d->name(), partList)
    , m_Device(d)
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
        for (const auto &p : LVM::pvList) { // FIXME: qAsConst
            if (p.isLuks())
                continue;
            if (p.vgName() == device()->name())
                dialogWidget().listPV().addPartition(*p.partition(), true);
            else if (p.vgName() == QString() && !LvmDevice::s_DirtyPVs.contains(p.partition())) // TODO: Remove LVM PVs in current VG
                dialogWidget().listPV().addPartition(*p.partition(), false);
        }
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
    targetPVList() << dialogWidget().listPV().checkedItems();
    QDialog::accept();
}
