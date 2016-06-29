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

#include "gui/createvolumedialog.h"
#include "gui/createvolumewidget.h"

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
CreateVolumeDialog::CreateVolumeDialog(QWidget* parent) :
    QDialog(parent),
    m_DialogWidget(new CreateVolumeWidget(this))
{
    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&dialogWidget());
    setWindowTitle(xi18nc("@title:window", "Cretae new Volume Group"));

    setupDialog();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), "createVolumeDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));

}

/** Destroys a CreateVolumeDialog */
CreateVolumeDialog::~CreateVolumeDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), "createVolumeDialog");
    kcg.writeEntry("Geometry", saveGeometry());
}

void CreateVolumeDialog::setupDialog()
{
    dialogButtonBox = new QDialogButtonBox;
    okButton = dialogButtonBox->addButton(QDialogButtonBox::Ok);
    cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
    mainLayout->addWidget(dialogButtonBox);
    okButton->setEnabled(false);
    cancelButton->setFocus();
    cancelButton->setDefault(true);
    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &CreateVolumeDialog::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &CreateVolumeDialog::reject);

    setMinimumSize(dialogWidget().size());
    resize(dialogWidget().size());
}

void CreateVolumeDialog::setupConnections()
{
}

