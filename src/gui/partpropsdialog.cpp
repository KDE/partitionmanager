/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2014 Yuri Chornoivan <yurchor@ukr.net>
    SPDX-FileCopyrightText: 2018 Abhijeet Sharma <sharma.abhijeet2096@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/partpropsdialog.h"
#include "gui/partpropswidget.h"

#include <core/partition.h>
#include <core/device.h>

#include <fs/filesystemfactory.h>

#include <util/capacity.h>
#include <util/helpers.h>
#include "util/guihelpers.h"

#include <QComboBox>
#include <QFontDatabase>
#include <QtGlobal>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

/** Creates a new PartPropsDialog
    @param parent pointer to the parent widget
    @param d the Device the Partition is on
    @param p the Partition to show properties for
*/
PartPropsDialog::PartPropsDialog(QWidget* parent, Device& d, Partition& p) :
    QDialog(parent),
    m_Device(d),
    m_Partition(p),
    m_WarnFileSystemChange(false),
    m_DialogWidget(new PartPropsWidget(this)),
    m_ReadOnly(partition().isMounted() || partition().state() == Partition::State::Copy || partition().state() == Partition::State::Restore || d.partitionTable()->isReadOnly()),
    m_ForceRecreate(false)
{
    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&dialogWidget());

    setWindowTitle(xi18nc("@title:window", "Partition properties: <filename>%1</filename>", partition().deviceNode()));

    setupDialog();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), "partPropsDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a PartPropsDialog */
PartPropsDialog::~PartPropsDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), "partPropsDialog");
    kcg.writeEntry("Geometry", saveGeometry());
}

/** @return the new label */
QString PartPropsDialog::newLabel() const
{
    return dialogWidget().label().text();
}

/** @return the new Partition flags */
PartitionTable::Flags PartPropsDialog::newFlags() const
{
    PartitionTable::Flags flags;

    for (int i = 0; i < dialogWidget().listFlags().count(); i++)
        if (dialogWidget().listFlags().item(i)->checkState() == Qt::Checked)
            flags |= static_cast<PartitionTable::Flag>(dialogWidget().listFlags().item(i)->data(Qt::UserRole).toInt());

    return flags;
}

/** @return the new FileSystem type */
FileSystem::Type PartPropsDialog::newFileSystemType() const
{
    return FileSystem::typeForName(dialogWidget().fileSystem().currentText());
}

void PartPropsDialog::setupDialog()
{
    dialogButtonBox = new QDialogButtonBox;
    okButton = dialogButtonBox->addButton(QDialogButtonBox::Ok);
    cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
    mainLayout->addWidget(dialogButtonBox);
    okButton->setEnabled(false);
    cancelButton->setFocus();
    cancelButton->setDefault(true);
    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &PartPropsDialog::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &PartPropsDialog::reject);

    dialogWidget().partWidget().init(&partition());

    const QString mp = partition().mountPoint().isEmpty()
                       ? xi18nc("@item mountpoint", "(none found)")
                       : partition().mountPoint();
    dialogWidget().mountPoint().setText(mp);

    dialogWidget().role().setText(partition().roles().toString());

    QString statusText = xi18nc("@label partition state", "idle");
    if (partition().isMounted()) {
        if (partition().roles().has(PartitionRole::Extended))
            statusText = xi18nc("@label partition state", "At least one logical partition is mounted.");
        else if (!partition().mountPoint().isEmpty())
            statusText = xi18nc("@label partition state", "mounted on <filename>%1</filename>", mp);
        else
            statusText = xi18nc("@label partition state", "mounted");
    }

    dialogWidget().status().setText(statusText);
    dialogWidget().uuid().setText(partition().fileSystem().uuid().isEmpty() ? xi18nc("@item uuid", "(none)") : partition().fileSystem().uuid());

    if(device().partitionTable()->type() == PartitionTable::gpt){
        QString PartitionLabel = partition().label().isEmpty() ? xi18nc("@item uuid", "(none)") : partition().label();
        QString PartitionUUID = partition().uuid().isEmpty() ? xi18nc("@item uuid", "(none)") : partition().uuid();

        dialogWidget().partitionLabel().setText(PartitionLabel);
        dialogWidget().partitionUuid().setText(PartitionUUID);
    }
    else{
        dialogWidget().partitionLabel().hide();
        dialogWidget().partitionTextLabel().hide();
        dialogWidget().partitionUuid().hide();
        dialogWidget().partitionTextUuid().hide();
    }

    setupFileSystemComboBox();

    // don't do this before the file system combo box has been set up!
    dialogWidget().label().setText(newLabel().isEmpty() ? partition().fileSystem().label() : newLabel());
    dialogWidget().capacity().setText(Capacity::formatByteSize(partition().capacity()));

    if (Capacity(partition(), Capacity::Type::Available).isValid()) {
        const qint64 availPercent = (partition().fileSystem().length() - partition().fileSystem().sectorsUsed()) * 100 / partition().fileSystem().length();

        const QString availString = QStringLiteral("%1% - %2")
                                    .arg(availPercent)
                                    .arg(Capacity::formatByteSize(partition().available()));
        const QString usedString = QStringLiteral("%1% - %2")
                                   .arg(100 - availPercent)
                                   .arg(Capacity::formatByteSize(partition().used()));

        dialogWidget().available().setText(availString);
        dialogWidget().used().setText(usedString);
    } else {
        dialogWidget().available().setText(Capacity::invalidString());
        dialogWidget().used().setText(Capacity::invalidString());
    }

    dialogWidget().firstSector().setText(QLocale().toString(partition().firstSector()));
    dialogWidget().lastSector().setText(QLocale().toString(partition().lastSector()));
    dialogWidget().numSectors().setText(QLocale().toString(partition().length()));

    setupFlagsList();

    updateHideAndShow();

    setMinimumSize(dialogWidget().size());
    resize(dialogWidget().size());
}

void PartPropsDialog::setupFlagsList()
{
    int f = 1;
    QString s;
    while (!(s = PartitionTable::flagName(static_cast<PartitionTable::Flag>(f))).isEmpty()) {
        if (partition().availableFlags() & f) {
            QListWidgetItem* item = new QListWidgetItem(s);
            dialogWidget().listFlags().addItem(item);
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setData(Qt::UserRole, f);
            item->setCheckState((partition().activeFlags() & f) ? Qt::Checked : Qt::Unchecked);
        }

        f <<= 1;
    }
}

void PartPropsDialog::updateHideAndShow()
{
    // create a temporary fs for some checks
    const FileSystem* fs = FileSystemFactory::create(newFileSystemType(), -1, -1, -1, -1, QString());

    if (fs == nullptr || fs->supportSetLabel() == FileSystem::cmdSupportNone) {
        dialogWidget().label().setReadOnly(true);
        dialogWidget().noSetLabel().setVisible(true);
        dialogWidget().noSetLabel().setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

        QPalette palette = dialogWidget().noSetLabel().palette();
        QColor f = palette.color(QPalette::Foreground);
        f.setAlpha(128);
        palette.setColor(QPalette::Foreground, f);
        dialogWidget().noSetLabel().setPalette(palette);
    } else {
        dialogWidget().label().setReadOnly(isReadOnly() && !partition().fileSystem().supportSetLabelOnline());
        dialogWidget().noSetLabel().setVisible(false);
    }

    // when do we show the uuid?
    const bool showUuid =
        partition().state() != Partition::State::New &&                           // not for new partitions
        !(fs == nullptr || fs->supportGetUUID() == FileSystem::cmdSupportNone);   // not if the FS doesn't support it

    dialogWidget().showUuid(showUuid);

    delete fs;

    // when do we show available and used capacity?
    const bool showAvailableAndUsed =
        partition().state() != Partition::State::New &&             // not for new partitions
        !partition().roles().has(PartitionRole::Extended) &&        // neither for extended
        !partition().roles().has(PartitionRole::Unallocated) &&     // or for unallocated
        newFileSystemType() != FileSystem::Type::Unformatted;       // and not for unformatted file systems

    dialogWidget().showAvailable(showAvailableAndUsed);
    dialogWidget().showUsed(showAvailableAndUsed);

    // when do we show the file system combo box?
    const bool showFileSystem =
        !partition().roles().has(PartitionRole::Extended) &&        // not for extended, they have no file system
        !partition().roles().has(PartitionRole::Unallocated) &&     // and not for unallocated: no choice there
                                                                    // do not show file system comboBox for open luks volumes.
        !(partition().roles().has(PartitionRole::Luks) && partition().fileSystem().type() != FileSystem::Type::Luks);
    dialogWidget().showFileSystem(showFileSystem);

    // when do we show the recreate file system check box?
    const bool showCheckRecreate =
        showFileSystem &&                                                       // only if we also show the file system
        partition().fileSystem().supportCreate() != FileSystem::cmdSupportNone &&  // and support creating this file system
        partition().fileSystem().type() != FileSystem::Type::Unknown &&         // and not for unknown file systems
        partition().state() != Partition::State::New &&                         // or new partitions
        !partition().roles().has(PartitionRole::Luks);                          // or encrypted filesystems

    dialogWidget().showCheckRecreate(showCheckRecreate);

    // when do we show the list of partition flags?
    const bool showListFlags =
        partition().state() != Partition::State::New &&                         // not for new partitions
        !partition().roles().has(PartitionRole::Unallocated);                   // and not for unallocated space

    dialogWidget().showListFlags(showListFlags);

    dialogWidget().checkRecreate().setEnabled(!isReadOnly());
    dialogWidget().listFlags().setEnabled(!isReadOnly());
    dialogWidget().fileSystem().setEnabled(!isReadOnly() && !forceRecreate());
}

void PartPropsDialog::setupConnections()
{
    connect(&dialogWidget().label(), &QLineEdit::textEdited, [this] (const QString &) {setDirty();});
    connect(&dialogWidget().fileSystem(), qOverload<int>(&QComboBox::currentIndexChanged), this, &PartPropsDialog::onFilesystemChanged);
    connect(&dialogWidget().checkRecreate(), &QCheckBox::stateChanged, this, &PartPropsDialog::onRecreate);

    // We want to enable the OK-button whenever the user checks or unchecks a flag in the flag list.
    // But it seems Qt doesn't offer a foolproof way to detect if this has happened: The currentRow/ItemChanged
    // signal only means the _current_ item has changed, but not necessarily that it was checked/unchecked. And
    // itemClicked alone isn't enough either. We choose to rather enable the OK-button too often than too
    // seldom.
    connect(&dialogWidget().listFlags(), &QListWidget::itemClicked, this, &PartPropsDialog::setDirty);
    connect(&dialogWidget().listFlags(), &QListWidget::currentRowChanged, [this] (int) {setDirty();});
}

void PartPropsDialog::setDirty(void*)
{
    okButton->setEnabled(true);
    okButton->setDefault(true);
}

void PartPropsDialog::setupFileSystemComboBox()
{
    dialogWidget().fileSystem().clear();
    QString selected;
    QStringList fsNames;

    for(const auto &fs : FileSystemFactory::map())
    {
        // If the partition isn't encrypted, skip the luks FS
        if (fs->type() == FileSystem::Type::Luks && partition().fileSystem().type() != FileSystem::Type::Luks)
            continue;
        if (fs->type() == FileSystem::Type::Luks2 && partition().fileSystem().type() != FileSystem::Type::Luks2)
            continue;
        if (partition().fileSystem().type() == fs->type() || (fs->supportCreate() != FileSystem::cmdSupportNone &&
                            partition().capacity() >= fs->minCapacity() && partition().capacity() <= fs->maxCapacity())) {
            QString name = fs->name();

            if (partition().fileSystem().type() == fs->type())
                selected = name;

            // If the partition isn't extended, skip the extended FS
            if (fs->type() == FileSystem::Type::Extended && !partition().roles().has(PartitionRole::Extended))
                continue;

            // The user cannot change the filesystem back to "unformatted" once a filesystem has been created.
            if (fs->type() == FileSystem::Type::Unformatted) {
                // .. but if the file system is unknown to us, show the unformatted option as the currently selected one
                if (partition().fileSystem().type() == FileSystem::Type::Unknown) {
                    name = FileSystem::nameForType(FileSystem::Type::Unformatted);
                    selected = name;
                } else if (partition().fileSystem().type() != FileSystem::Type::Unformatted && partition().state() != Partition::State::New)
                    continue;
            }

            fsNames.append(name);
        }
    }

    std::sort(fsNames.begin(), fsNames.end(), caseInsensitiveLessThan);

    for (const auto &fsName : qAsConst(fsNames))
        dialogWidget().fileSystem().addItem(createFileSystemColor(FileSystem::typeForName(fsName), 8), fsName);

    dialogWidget().fileSystem().setCurrentIndex(dialogWidget().fileSystem().findText(selected));

    const FileSystem* fs = FileSystemFactory::create(FileSystem::typeForName(dialogWidget().fileSystem().currentText()), -1, -1, -1, -1, QString());
    dialogWidget().m_EditLabel->setMaxLength(fs->maxLabelLength());
    dialogWidget().m_EditLabel->setValidator(fs->labelValidator(dialogWidget().m_EditLabel));
}

void PartPropsDialog::updatePartitionFileSystem()
{
    FileSystem* fs = FileSystemFactory::create(newFileSystemType(), partition().firstSector(), partition().lastSector(), partition().sectorSize());
    partition().deleteFileSystem();
    partition().setFileSystem(fs);
    dialogWidget().partWidget().update();
}

void PartPropsDialog::onFilesystemChanged(int)
{
    if (partition().state() == Partition::State::New || warnFileSystemChange() || KMessageBox::warningContinueCancel(this,
            xi18nc("@info", "<para><warning>You are about to lose all data on partition <filename>%1</filename>.</warning></para>"
                   "<para>Changing the file system on a partition already on disk will erase all its contents. If you continue now and apply the resulting operation in the main window, all data on <filename>%1</filename> will unrecoverably be lost.</para>", partition().deviceNode()),
            xi18nc("@title:window", "Really Recreate <filename>%1</filename> with File System %2?", partition().deviceNode(), dialogWidget().fileSystem().currentText()),
            KGuiItem(xi18nc("@action:button", "Change the File System"), QStringLiteral("arrow-right")),
            KGuiItem(xi18nc("@action:button", "Do Not Change the File System"), QStringLiteral("dialog-cancel")), QStringLiteral("reallyChangeFileSystem")) == KMessageBox::Continue) {
        setDirty();
        updateHideAndShow();
        setWarnFileSystemChange();
        updatePartitionFileSystem();

        const FileSystem* fs = FileSystemFactory::create(FileSystem::typeForName(dialogWidget().fileSystem().currentText()), -1, -1, -1, -1, QString());
        dialogWidget().m_EditLabel->setMaxLength(fs->maxLabelLength());
        dialogWidget().m_EditLabel->setValidator(fs->labelValidator(dialogWidget().m_EditLabel));
    } else {
        dialogWidget().fileSystem().disconnect(this);
        setupFileSystemComboBox();
        connect(&dialogWidget().fileSystem(), qOverload<int>(&QComboBox::currentIndexChanged), this, &PartPropsDialog::onFilesystemChanged);
    }
}

void PartPropsDialog::onRecreate(int state)
{
    if (state == Qt::Checked && (warnFileSystemChange() || KMessageBox::warningContinueCancel(this,
                                 xi18nc("@info", "<para><warning>You are about to lose all data on partition <filename>%1</filename>.</warning></para>"
                                        "<para>Recreating a file system will erase all its contents. If you continue now and apply the resulting operation in the main window, all data on <filesystem>%1</filesystem> will unrecoverably be lost.</para>", partition().deviceNode()),
                                 xi18nc("@title:window", "Really Recreate File System on <filename>%1</filename>?", partition().deviceNode()),
                                 KGuiItem(xi18nc("@action:button", "Recreate the File System"), QStringLiteral("arrow-right")),
                                 KGuiItem(xi18nc("@action:button", "Do Not Recreate the File System"), QStringLiteral("dialog-cancel")), QStringLiteral("reallyRecreateFileSystem")) == KMessageBox::Continue)) {
        setDirty();
        setWarnFileSystemChange();
        setForceRecreate(true);
        dialogWidget().fileSystem().setCurrentIndex(dialogWidget().fileSystem().findText(partition().fileSystem().name()));
        dialogWidget().fileSystem().setEnabled(false);
        updateHideAndShow();
        updatePartitionFileSystem();
    } else {
        setForceRecreate(false);
        dialogWidget().checkRecreate().setCheckState(Qt::Unchecked);
        dialogWidget().fileSystem().setEnabled(true);
        updateHideAndShow();
    }
}
