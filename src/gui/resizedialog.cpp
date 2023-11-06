/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/resizedialog.h"
#include "gui/sizedialogwidget.h"
#include "gui/sizedetailswidget.h"

#include <core/partition.h>
#include <core/device.h>

#include <fs/filesystem.h>
#include <fs/luks2.h>

#include <ops/resizeoperation.h>

#include <util/capacity.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPasswordDialog>
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

    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("resizeDialog"));
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a ResizeDialog */
ResizeDialog::~ResizeDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("resizeDialog"));
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
    if (partition().roles().has(PartitionRole::Luks)) {
        FS::luks2* luksFs = dynamic_cast<FS::luks2*>(&partition().fileSystem());
        if (luksFs) {
            if (luksFs->keyLocation() == FS::luks::KeyLocation::keyring) {
                bool validPassphrase = false;
                QString errorMessage;
                QString passphrase;

                while  (!validPassphrase) {
                    KPasswordDialog dlg( this );
                    dlg.setPrompt(i18nc("%2 is either empty or says Invalid passphrase.", "%2Enter passphrase for %1:", partition().deviceNode(), errorMessage));
                    if (!dlg.exec()) {
                        reject();
                        return;
                    }

                    passphrase = dlg.password();
                    validPassphrase = luksFs->testPassphrase(partition().deviceNode(), passphrase);
                    if(!validPassphrase)
                        errorMessage = i18nc("Part of %2Enter passphrase for %1:", "Invalid passphrase. ");
                }
                luksFs->setPassphrase(passphrase);
            }
        }
    }

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
    if (device().type() == Device::Type::LVM_Device) {
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
    return (device().type() == Device::Type::LVM_Device) ? false : ResizeOperation::canMove(&partition());
}
