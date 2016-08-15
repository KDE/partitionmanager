/*************************************************************************
 *  Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                *
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

#include "gui/resizedialog.h"
#include "gui/sizedialogwidget.h"
#include "gui/sizedetailswidget.h"

#include <core/partition.h>
#include <core/device.h>

#include <fs/filesystem.h>

#include <ops/resizeoperation.h>

#include <util/capacity.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/** Creates a new ResizeDialog
    @param parent pointer to the parent widget
    @param d the Device the Partition to resize is on
    @param p the Partition to resize
    @param minFirst the first free sector before the Partition to resize
    @param maxLast the last free sector free after the Partition to resize
*/
ResizeDialog::ResizeDialog(QWidget* parent, Device& d, Partition& p, qint64 minFirst, qint64 maxLast) :
    SizeDialogBase(parent, d, p, minFirst, maxLast),
    m_OriginalFirstSector(p.firstSector()),
    m_OriginalLastSector(p.lastSector()),
    m_ResizedFirstSector(p.firstSector()),
    m_ResizedLastSector(p.lastSector())
{
    setWindowTitle(xi18nc("@title:window", "Resize/move partition: <filename>%1</filename>", partition().deviceNode()));

    dialogWidget().hideRole();
    dialogWidget().hideFileSystem();
    dialogWidget().hideLabel();
    dialogWidget().textLVName().hide();
    dialogWidget().lvName().hide();


    setupDialog();
    setupConstraints();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), "resizeDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a ResizeDialog */
ResizeDialog::~ResizeDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), "resizeDialog");
    kcg.writeEntry("Geometry", saveGeometry());
}

void ResizeDialog::rollback()
{
    partition().setFirstSector(originalFirstSector());
    partition().fileSystem().setFirstSector(originalFirstSector());

    partition().setLastSector(originalLastSector());
    partition().fileSystem().setLastSector(originalLastSector());

    if (partition().roles().has(PartitionRole::Extended)) {
        device().partitionTable()->removeUnallocated(&partition());
        device().partitionTable()->insertUnallocated(device(), &partition(), partition().firstSector());
    }
}

void ResizeDialog::accept()
{
    setResizedFirstSector(partition().firstSector());
    setResizedLastSector(partition().lastSector());

    rollback();
    QDialog::accept();
}

void ResizeDialog::reject()
{
    rollback();
    QDialog::reject();
}

void ResizeDialog::setupDialog()
{
    SizeDialogBase::setupDialog();
    if (device().type() == Device::LVM_Device) {
        dialogWidget().hideBeforeAndAfter();
        detailsWidget().checkAlign().setChecked(false);
        detailsWidget().checkAlign().setEnabled(false);
        detailsButton->hide();
    }
    okButton->setEnabled(false);
}

void ResizeDialog::setDirty()
{
    okButton->setEnabled(isModified());
}

/** @return true if the user modified anything */
bool ResizeDialog::isModified() const
{
    return partition().firstSector() != originalFirstSector() || partition().lastSector() != originalLastSector();
}

bool ResizeDialog::canGrow() const
{
    return ResizeOperation::canGrow(&partition());
}

bool ResizeDialog::canShrink() const
{
    return ResizeOperation::canShrink(&partition());
}

bool ResizeDialog::canMove() const
{
    return (device().type() == Device::LVM_Device) ? false : ResizeOperation::canMove(&partition());
}
