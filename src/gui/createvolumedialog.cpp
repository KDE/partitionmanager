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
#include "gui/createvolumedialog.h"

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
#include <QDialogButtonBox>

CreateVolumeDialog::CreateVolumeDialog(QWidget* parent, QString& vgname, QStringList& pvlist, qint32& pesize) :
    VolumeDialog(parent, vgname, pvlist),
    m_PESize(pesize),
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

void CreateVolumeDialog::setupDialog()
{
    foreach (QString pvpath, FS::lvm2_pv::getFreePV()) {
        if (!LvmDevice::s_DirtyPVs.contains(pvpath)) {
            dialogWidget().listPV().addPartition(pvpath, false);
        }
    }
}

void CreateVolumeDialog::setupConnections()
{
    connect(&dialogWidget().vgName(), &QLineEdit::textChanged, this, &CreateVolumeDialog::onVGNameChanged);
    connect(&dialogWidget().spinPESize(), static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CreateVolumeDialog::onSpinPESizeChanged);
}

void  CreateVolumeDialog::accept()
{
    QString& tname = targetName();
    tname = dialogWidget().vgName().text();

    targetPVList() << dialogWidget().listPV().checkedItems();

    qint32& pesize = peSize();
    pesize = dialogWidget().spinPESize().value();

    QDialog::accept();
}

void CreateVolumeDialog::onVGNameChanged(const QString& vgname)
{
    if (m_SystemVGList.contains(vgname)) {
        m_IsValidName = false;
    } else {
        m_IsValidName = true;
    }
    updateOkButtonStatus();
}

void CreateVolumeDialog::onSpinPESizeChanged(int newsize)
{
    Q_UNUSED(newsize);
    updateOkButtonStatus();
    updateSectorInfos();
}
