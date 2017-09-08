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

#include "gui/createvolumegroupdialog.h"
#include "gui/volumegroupwidget.h"

#include <core/lvmdevice.h>

#include <fs/lvm2_pv.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

CreateVolumeGroupDialog::CreateVolumeGroupDialog(QWidget* parent, QString& vgName, QVector<const Partition*>& partList, qint32& peSize, QList<Device*> devices)
    : VolumeGroupDialog(parent, vgName, partList)
    , m_PESize(peSize)
    , m_Devices(devices)
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
    for (const auto &p : qAsConst(LVM::pvList))
        if (!p.isLuks() && p.vgName() == QString() && !LvmDevice::s_DirtyPVs.contains(p.partition()))
            dialogWidget().listPV().addPartition(*p.partition(), false);
}

void CreateVolumeGroupDialog::setupConnections()
{
    connect(&dialogWidget().vgName(), &QLineEdit::textChanged, this, &CreateVolumeGroupDialog::onVGNameChanged);
    connect(&dialogWidget().spinPESize(), static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CreateVolumeGroupDialog::onSpinPESizeChanged);
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
