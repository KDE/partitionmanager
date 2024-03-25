/*
    SPDX-FileCopyrightText: 2008-2012 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2013-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/newdialog.h"
#include "gui/sizedialogwidget.h"
#include "gui/sizedetailswidget.h"

#include <core/partition.h>
#include <core/device.h>

#include <fs/filesystemfactory.h>
#include <fs/luks.h>

#include <util/capacity.h>
#include <util/helpers.h>
#include "util/guihelpers.h"

#include <utility>

#include <QtGlobal>
#include <QFontDatabase>
#include <QPalette>

#include <KColorScheme>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/** Creates a NewDialog
    @param parent the parent widget
    @param device the Device on which a new Partition is to be created
    @param unallocatedPartition the unallocated space on the Device to create a Partition in
    @param r the permitted Roles for the new Partition
*/
NewDialog::NewDialog(QWidget* parent, Device& device, Partition& unallocatedPartition, PartitionRole::Roles r) :
    SizeDialogBase(parent, device, unallocatedPartition, unallocatedPartition.firstSector(), unallocatedPartition.lastSector()),
    m_PartitionRoles(r),
    m_IsValidPassword(true)
{
    setWindowTitle(xi18nc("@title:window", "Create a new partition"));

    setupDialog();
    setupConstraints();
    setupConnections();

    updateOkButtonStatus();

    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("newDialog"));
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

NewDialog::~NewDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("newDialog"));
    kcg.writeEntry("Geometry", saveGeometry());
}

void NewDialog::setupDialog()
{
    QStringList fsNames;
    for (const auto &fs : FileSystemFactory::map()) {
        if (fs->supportCreate() != FileSystem::cmdSupportNone &&
            fs->type() != FileSystem::Type::Extended &&
            fs->type() != FileSystem::Type::Luks &&
            fs->type() != FileSystem::Type::Luks2)
            fsNames.append(fs->name());
    }

    std::sort(fsNames.begin(), fsNames.end(), caseInsensitiveLessThan);

    for (const auto &fsName : std::as_const(fsNames))
        dialogWidget().comboFileSystem().addItem(createFileSystemColor(FileSystem::typeForName(fsName), 8), fsName);

    QString selected = FileSystem::nameForType(GuiHelpers::defaultFileSystem());
    const int idx = dialogWidget().comboFileSystem().findText(selected);
    dialogWidget().comboFileSystem().setCurrentIndex(idx != -1 ? idx : 0);

    dialogWidget().radioPrimary().setVisible(partitionRoles() & PartitionRole::Primary);
    dialogWidget().radioExtended().setVisible(partitionRoles() & PartitionRole::Extended);
    dialogWidget().radioLogical().setVisible(partitionRoles() & PartitionRole::Logical);

    if (partitionRoles() & PartitionRole::Primary)
        dialogWidget().radioPrimary().setChecked(true);
    else
        dialogWidget().radioLogical().setChecked(true);

    SizeDialogBase::setupDialog();

    dialogWidget().checkBoxEncrypt().hide();
    dialogWidget().checkBoxLuks2().hide();
    dialogWidget().editPassphrase().hide();

    if (device().type() == Device::Type::LVM_Device) {
        dialogWidget().comboFileSystem().removeItem(dialogWidget().comboFileSystem().findText(QStringLiteral("lvm2 pv")));
    }


    dialogWidget().editPassphrase().setMinimumPasswordLength(1);
    dialogWidget().editPassphrase().setMaximumPasswordLength(512); // cryptsetup does not support longer passwords

    // set a background warning color (taken from the current color scheme)
    KColorScheme colorScheme(QPalette::Active, KColorScheme::View);
    dialogWidget().editPassphrase().setBackgroundWarningColor(colorScheme.background(KColorScheme::NegativeBackground).color());


    // don't move these above the call to parent's setupDialog, because only after that has
    // run there is a valid partition set in the part resizer widget and they will need that.
    onRoleChanged(false);
    onFilesystemChanged(dialogWidget().comboFileSystem().currentIndex());

    auto showPermissionsGroup = [this] {
        const QString currText = dialogWidget().comboFileSystem().currentText();
        const bool enablePosixPermission = QList<QString>({
                QStringLiteral("bcachefs"),
                QStringLiteral("btrfs"),
                QStringLiteral("ext2"),
                QStringLiteral("ext3"),
                QStringLiteral("ext4"),
                QStringLiteral("f2fs"),
                QStringLiteral("hfsplus"),
                QStringLiteral("jfs"),
                QStringLiteral("minix"),
                QStringLiteral("ocfs2"),
                QStringLiteral("reiserfs"),
                QStringLiteral("reiser4"),
                QStringLiteral("udf"),
                QStringLiteral("xfs"),
                QStringLiteral("zfs"),}
            ).contains(currText);
        if (enablePosixPermission) {
            dialogWidget().showPosixPermissions();
        } else {
            dialogWidget().hidePosixPermissions();
        }
    };
    connect(&dialogWidget().comboFileSystem(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, showPermissionsGroup);
    showPermissionsGroup();
    dialogWidget().radioRootPermissions().setChecked(true);
}

void NewDialog::setupConnections()
{
    connect(&dialogWidget().radioPrimary(), &QRadioButton::toggled, this, &NewDialog::onRoleChanged);
    connect(&dialogWidget().radioExtended(), &QRadioButton::toggled, this, &NewDialog::onRoleChanged);
    connect(&dialogWidget().radioLogical(), &QRadioButton::toggled, this, &NewDialog::onRoleChanged);
    connect(&dialogWidget().checkBoxEncrypt(), &QCheckBox::toggled, this, &NewDialog::onRoleChanged);
    connect(&dialogWidget().comboFileSystem(), &QComboBox::currentIndexChanged, this, &NewDialog::onFilesystemChanged);
    connect(&dialogWidget().label(), &QLineEdit::textChanged, this, &NewDialog::onLabelChanged);
    // listen to password status updates
    connect(&dialogWidget().editPassphrase(), &KNewPasswordWidget::passwordStatusChanged, this, &NewDialog::slotPasswordStatusChanged);

    SizeDialogBase::setupConnections();
}

bool NewDialog::canMove() const
{
    return (device().type() == Device::Type::LVM_Device) ? false : true;
}

void NewDialog::accept()
{
    if (partition().roles().has(PartitionRole::Extended)) {
        partition().deleteFileSystem();
        partition().setFileSystem(FileSystemFactory::create(FileSystem::Type::Extended,
                                                            partition().firstSector(),
                                                            partition().lastSector(),
                                                            partition().sectorSize()));
    }
    else if (partition().roles().has(PartitionRole::Luks)) {
        FileSystem::Type innerFsType = partition().fileSystem().type();
        partition().deleteFileSystem();
        bool isLuks2 = dialogWidget().checkBoxLuks2().isVisible() && dialogWidget().checkBoxLuks2().isChecked();
        FS::luks* luksFs = dynamic_cast< FS::luks* >(
                               FileSystemFactory::create(isLuks2 ? FileSystem::Type::Luks2 : FileSystem::Type::Luks,
                                                         partition().firstSector(),
                                                         partition().lastSector(),
                                                         partition().sectorSize()));
        luksFs->createInnerFileSystem(innerFsType);
        luksFs->setPassphrase(dialogWidget().editPassphrase().password());
        partition().setFileSystem(luksFs);
        partition().fileSystem().setLabel(dialogWidget().label().text());
    }

    QDialog::accept();
}

void NewDialog::onRoleChanged(bool)
{
    PartitionRole::Roles r = PartitionRole::None;

    if (dialogWidget().radioPrimary().isChecked())
        r = PartitionRole::Primary;
    else if (dialogWidget().radioExtended().isChecked())
        r = PartitionRole::Extended;
    else if (dialogWidget().radioLogical().isChecked())
        r = PartitionRole::Logical;

    bool doEncrypt = dialogWidget().checkBoxEncrypt().isVisible() &&
                       dialogWidget().checkBoxEncrypt().isChecked();
    if (doEncrypt)
        r |= PartitionRole::Luks;

    dialogWidget().checkBoxLuks2().setVisible(doEncrypt);
    dialogWidget().editPassphrase().setVisible(doEncrypt);

    // Make sure an extended partition gets correctly displayed: Set its file system to extended.
    // Also make sure to set a primary's or logical's file system once the user goes back from
    // extended to any of those.
    if (r == PartitionRole::Extended)
        updateFileSystem(FileSystem::Type::Extended);
    else
        updateFileSystem(FileSystem::typeForName(dialogWidget().comboFileSystem().currentText()));

    dialogWidget().comboFileSystem().setEnabled(r != PartitionRole::Extended);
    partition().setRoles(PartitionRole(r));

    setupConstraints();

    dialogWidget().partResizerWidget().resizeLogicals(0, 0, true);
    dialogWidget().partResizerWidget().update();

    updateHideAndShow();
}

void NewDialog::updateFileSystem(FileSystem::Type t)
{
    partition().deleteFileSystem();
    partition().setFileSystem(FileSystemFactory::create(t, partition().firstSector(), partition().lastSector(), partition().sectorSize()));
}

void NewDialog::onFilesystemChanged(int idx)
{
    updateFileSystem(FileSystem::typeForName(dialogWidget().comboFileSystem().itemText(idx)));

    setupConstraints();
    updateOkButtonStatus();

    const FileSystem* fs = FileSystemFactory::create(FileSystem::typeForName(dialogWidget().comboFileSystem().currentText()), -1, -1, -1, -1, QString());
    dialogWidget().m_EditLabel->setMaxLength(fs->maxLabelLength());
    dialogWidget().m_EditLabel->setValidator(fs->labelValidator(dialogWidget().m_EditLabel));

    updateSpinCapacity(partition().length());
    dialogWidget().partResizerWidget().update();

    updateHideAndShow();
}

void NewDialog::onLabelChanged(const QString& newLabel)
{
    partition().fileSystem().setLabel(newLabel);
}

void NewDialog::slotPasswordStatusChanged()
{
    // You may want to extend this switch with more cases,
    // in order to warn the user about all the possible password issues.
    switch (dialogWidget().editPassphrase().passwordStatus()) {
    case KNewPasswordWidget::WeakPassword:
    case KNewPasswordWidget::StrongPassword:
        m_IsValidPassword = true;
        break;
    default:
        m_IsValidPassword = false;
        break;
    }
    updateOkButtonStatus();
}

void NewDialog::updateHideAndShow()
{
    // this is mostly copy'n'pasted from PartPropsDialog::updateHideAndShow()
    if (partition().roles().has(PartitionRole::Extended) ||
       (partition().fileSystem().supportSetLabel() == FileSystem::cmdSupportNone &&
        partition().fileSystem().supportCreateWithLabel() == FileSystem::cmdSupportNone) )
    {
        dialogWidget().label().setReadOnly(true);
        dialogWidget().noSetLabel().setVisible(true);
        dialogWidget().noSetLabel().setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

        QPalette palette = dialogWidget().noSetLabel().palette();
        QColor f = palette.color(QPalette::WindowText);
        f.setAlpha(128);
        palette.setColor(QPalette::WindowText, f);
        dialogWidget().noSetLabel().setPalette(palette);
        dialogWidget().checkBoxEncrypt().hide();
        dialogWidget().editPassphrase().hide();
    } else {
        dialogWidget().label().setReadOnly(false);
        dialogWidget().noSetLabel().setVisible(false);
    }
    if (FileSystemFactory::map()[FileSystem::Type::Luks]->supportCreate() && FS::luks::canEncryptType(FileSystem::typeForName(dialogWidget().comboFileSystem().currentText())) && !partition().roles().has(PartitionRole::Extended))
    {
        dialogWidget().checkBoxEncrypt().show();
        if (dialogWidget().checkBoxEncrypt().isChecked())
        {
            dialogWidget().editPassphrase().show();
            slotPasswordStatusChanged();
        }
    }
    else
    {
        dialogWidget().checkBoxEncrypt().hide();
        dialogWidget().editPassphrase().hide();
    }

}

void NewDialog::updateOkButtonStatus()
{
    okButton->setEnabled(isValidPassword() && isValidLVName());
}

bool NewDialog::useUnsecuredPartition() const
{
    return !dialogWidget().isPermissionOnlyRoot();
}
