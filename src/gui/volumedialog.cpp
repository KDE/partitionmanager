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

#include <core/partitiontable.h>
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

/** Creates a new VolumeDialog
    @param parent pointer to the parent widget
    @param d the Device to show properties for
*/
VolumeDialog::VolumeDialog(QWidget* parent, QString& vgname, QStringList& pvlist) :
    QDialog(parent),
    m_DialogWidget(new VolumeWidget(this)),
    m_TargetName(vgname),
    m_TargetPVList(pvlist)
{
    Q_UNUSED(pvlist);
    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&dialogWidget());

    dialogButtonBox = new QDialogButtonBox;
    okButton = dialogButtonBox->addButton(QDialogButtonBox::Ok);
    cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
    mainLayout->addWidget(dialogButtonBox);
    updateOkButtonStatus();
    cancelButton->setFocus();
    cancelButton->setDefault(true);

    setupDialog();
    setupConstraints();
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
    dialogWidget().vgName().setText(targetName());

    dialogWidget().volumeType().addItem(QStringLiteral("LVM"));
    dialogWidget().volumeType().addItem(QStringLiteral("RAID"));
    dialogWidget().volumeType().setCurrentIndex(0);

    setMinimumSize(dialogWidget().size());
    resize(dialogWidget().size());

    updatePartTable();
}

void VolumeDialog::setupConnections()
{
    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &VolumeDialog::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &VolumeDialog::reject);
    connect(&dialogWidget().volumeType(), static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &VolumeDialog::onVolumeTypeChanged);
}

void VolumeDialog::setupConstraints()
{
    updatePartTable();
    updateSizeInfos();
    updateOkButtonStatus();
}

void VolumeDialog::updateOkButtonStatus()
{
    bool enable = true;

    if (dialogWidget().vgName().text().isEmpty()) {
        enable = false;
    }
    if (dialogWidget().spinPESize().value() <= 0) {
        enable = false;
    }

    okButton->setEnabled(enable);
}

void VolumeDialog::updatePartTable()
{
}

void VolumeDialog::updateSizeInfos()
{
}

void VolumeDialog::updatePartitionList()
{
}

void VolumeDialog::onPartitionListChanged()
{
}


void VolumeDialog::onVolumeTypeChanged(int index)
{
    updatePartitionList();
}
