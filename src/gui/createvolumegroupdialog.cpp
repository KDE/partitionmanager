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

#include "gui/createvolumegroupdialog.h"
#include "gui/volumegroupwidget.h"

#include <core/lvmdevice.h>
#include <fs/lvm2_pv.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

CreateVolumeGroupDialog::CreateVolumeGroupDialog(QWidget* parent, const QList<Device*>& devices, QString& vgName, QStringList& pvList, qint32& peSize) :
    VolumeGroupDialog(parent, vgName, pvList),
    m_PESize(peSize),
    m_Devices(devices),
    m_SystemVGList(LvmDevice::getVGs())
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
    for (const auto &p : FS::lvm2_pv::getPVs(m_Devices)) {
        if (!LvmDevice::s_DirtyPVs.contains(p->deviceNode())) {
            dialogWidget().listPV().addPartition(p->deviceNode(), false);
        }
    }
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

    targetPVList() << dialogWidget().listPV().checkedItems();

    qint32& pesize = peSize();
    pesize = dialogWidget().spinPESize().value();

    QDialog::accept();
}

void CreateVolumeGroupDialog::onVGNameChanged(const QString& vgname)
{
    if (m_SystemVGList.contains(vgname)) {
        m_IsValidName = false;
    } else {
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
