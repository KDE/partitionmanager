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

#include "gui/volumedialog.h"
#include "gui/volumewidget.h"

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

/** Creates a new VolumeDialog
    @param parent pointer to the parent widget
    @param d the Device to show properties for
*/
VolumeDialog::VolumeDialog(QWidget* parent) :
    QDialog(parent),
    m_DialogWidget(new VolumeWidget(this))
{
    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&dialogWidget());

    dialogButtonBox = new QDialogButtonBox;
    okButton = dialogButtonBox->addButton(QDialogButtonBox::Ok);
    cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
    mainLayout->addWidget(dialogButtonBox);
    okButton->setEnabled(false);
    cancelButton->setFocus();
    cancelButton->setDefault(true);
    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &VolumeDialog::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &VolumeDialog::reject);

    setupDialog();
    setupConnections();
}

/** Destroys a VolumeDialog */
VolumeDialog::~VolumeDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), "createVolumeDialog");
    kcg.writeEntry("Geometry", saveGeometry());
}

void VolumeDialog::setupDialog()
{
    setMinimumSize(dialogWidget().size());
    resize(dialogWidget().size());
}

void VolumeDialog::setupConnections()
{
}
