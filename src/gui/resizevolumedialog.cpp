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
#include <fs/lvm2_pv.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KIconLoader>

#include <QPointer>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QDialogButtonBox>

/** Creates a new ResizeVolumeDialog
    @param parent pointer to the parent widget
    @param d the Device to show properties for
*/
ResizeVolumeDialog::ResizeVolumeDialog(QWidget* parent, QString& vgname, QStringList& partlist) :
    VolumeDialog(parent, vgname, partlist)
{
    setWindowTitle(xi18nc("@title:window", "Resize Volume Group"));

    setupDialog();
    setupConstraints();

    KConfigGroup kcg(KSharedConfig::openConfig(), "resizeVolumeDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

void ResizeVolumeDialog::setupDialog()
{
    dialogWidget().listPV().addPartitionList(LvmDevice::getPVs(targetName()), true);
    dialogWidget().listPV().addPartitionList(FS::lvm2_pv::getFreePV(), false);
}

void ResizeVolumeDialog::setupConstraints()
{
    dialogWidget().vgName().setEnabled(false);
    dialogWidget().spinPESize().setEnabled(false);
    dialogWidget().volumeType().setEnabled(false);
}

void ResizeVolumeDialog::accept()
{
    targetPVList() << dialogWidget().listPV().checkedItems();
    QDialog::accept();
}

