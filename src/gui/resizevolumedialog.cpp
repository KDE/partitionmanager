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

#include "gui/volumewidget.h"
#include "gui/volumedialog.h"
#include "gui/resizevolumedialog.h"

#include <core/lvmdevice.h>
#include <core/volumemanagerdevice.h>
#include <core/partitiontable.h>
#include <fs/lvm2_pv.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/** Creates a new ResizeVolumeDialog
    @param parent pointer to the parent widget
    @param dev the Device to show properties for
*/
ResizeVolumeDialog::ResizeVolumeDialog(QWidget* parent, QString& vgname, QStringList& partlist, VolumeManagerDevice& dev) :
    VolumeDialog(parent, vgname, partlist),
    m_Device(dev)
{
    setWindowTitle(xi18nc("@title:window", "Resize Volume Group"));

    setupDialog();
    setupConstraints();

    KConfigGroup kcg(KSharedConfig::openConfig(), "resizeVolumeDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

void ResizeVolumeDialog::setupDialog()
{
    if (dialogWidget().volumeType().currentText() == QStringLiteral("LVM")) {
        dialogWidget().listPV().addPartitionList(device().deviceNodes(), true);
        for (const auto &pvpath : FS::lvm2_pv::getFreePV()) {
            if (!LvmDevice::s_DirtyPVs.contains(pvpath)) {
                dialogWidget().listPV().addPartition(pvpath, false);
            }
        }
    }
}

void ResizeVolumeDialog::setupConstraints()
{
    dialogWidget().vgName().setEnabled(false);
    dialogWidget().spinPESize().setEnabled(false);
    dialogWidget().volumeType().setEnabled(false);
    VolumeDialog::setupConstraints();
}

void ResizeVolumeDialog::accept()
{
    targetPVList() << dialogWidget().listPV().checkedItems();
    QDialog::accept();
}
