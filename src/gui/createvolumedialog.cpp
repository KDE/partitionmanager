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

/** Creates a new CreateVolumeDialog
    @param parent pointer to the parent widget
    @param d the Device to show properties for
*/
CreateVolumeDialog::CreateVolumeDialog(QWidget* parent, QString& vgname, QList<Partition*>& pvlist) :
    VolumeDialog(parent, vgname, pvlist)
{
    setWindowTitle(xi18nc("@title:window", "Cretae new Volume Group"));

    setupDialog();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), "createVolumeDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a CreateVolumeDialog */
CreateVolumeDialog::~CreateVolumeDialog()
{
}

void CreateVolumeDialog::setupDialog()
{
    VolumeDialog::setupDialog();
}

void CreateVolumeDialog::setupConstraints()
{
    VolumeDialog::setupConstraints();
}

void CreateVolumeDialog::setupConnections()
{
    VolumeDialog::setupConnections();
    connect(&dialogWidget().vgName(), &QLineEdit::textChanged, this, &CreateVolumeDialog::onVGNameChanged);
    connect(&dialogWidget().spinPESize(), static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CreateVolumeDialog::onSpinPESizeChanged);
}

void  CreateVolumeDialog::accept()
{
    QDialog::accept();
}

void CreateVolumeDialog::onVGNameChanged(const QString& vgname)
{
    Q_UNUSED(vgname);
    updateOkButtonStatus();
}

void CreateVolumeDialog::onSpinPESizeChanged(int newsize)
{
    Q_UNUSED(newsize);
    updateOkButtonStatus();
}
