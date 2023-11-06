/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/filesystemsupportdialog.h"
#include "gui/filesystemsupportdialogwidget.h"

#include <fs/filesystem.h>
#include <fs/filesystemfactory.h>

#include <QDialogButtonBox>
#include <QDialog>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/** Creates a new FileSystemSupportDialog
    @param parent the parent object
*/
FileSystemSupportDialog::FileSystemSupportDialog(QWidget* parent) :
    QDialog(parent),
    m_FileSystemSupportDialogWidget(new FileSystemSupportDialogWidget(this))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&dialogWidget());
    setWindowTitle(xi18nc("@title:window", "File System Support"));
    dialogButtonBox = new QDialogButtonBox(this);
    dialogButtonBox -> setStandardButtons(QDialogButtonBox::Ok);
    mainLayout->addWidget(dialogButtonBox);

    setupDialog();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("fileSystemSupportDialog"));
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a FileSystemSupportDialog */
FileSystemSupportDialog::~FileSystemSupportDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("fileSystemSupportDialog"));
    kcg.writeEntry("Geometry", saveGeometry());
}

QSize FileSystemSupportDialog::sizeHint() const
{
    return QSize(690, 490);
}

void FileSystemSupportDialog::setupDialog()
{
    QIcon yes = QIcon::fromTheme(QStringLiteral("data-success"));
    QIcon no = QIcon::fromTheme(QStringLiteral("data-error"));

    dialogWidget().tree().clear();

    for (const auto &fs : FileSystemFactory::map()) {
        if (fs->type() == FileSystem::Type::Unknown || fs->type() == FileSystem::Type::Extended ||
            fs->type() == FileSystem::Type::Luks || fs->type() == FileSystem::Type::Luks2) {
            continue;
        }

        QTreeWidgetItem* item = new QTreeWidgetItem();

        int i = 0;
        item->setText(i++, fs->name());
        item->setIcon(i++, fs->supportCreate() ? yes : no);
        item->setIcon(i++, fs->supportGrow() ? yes : no);
        item->setIcon(i++, fs->supportShrink() ? yes : no);
        item->setIcon(i++, fs->supportMove() ? yes : no);
        item->setIcon(i++, fs->supportCopy() ? yes : no);
        item->setIcon(i++, fs->supportCheck() ? yes : no);
        item->setIcon(i++, fs->supportGetLabel() ? yes : no);
        item->setIcon(i++, fs->supportSetLabel() ? yes : no);
        item->setIcon(i++, fs->supportGetUsed() ? yes : no);
        item->setIcon(i++, fs->supportBackup() ? yes : no);

        // there is no FileSystem::supportRestore(), because we currently can't tell
        // if a file is an image of a supported or unsupported (or even invalid) filesystem
        item->setIcon(i++, yes);

        item->setText(i++, fs->supportToolName().name.isEmpty() ? QStringLiteral("---") : fs->supportToolName().name);

        dialogWidget().tree().addTopLevelItem(item);
    }

    for (int i = 0; i < dialogWidget().tree().columnCount(); i++)
        dialogWidget().tree().resizeColumnToContents(i);

    dialogWidget().tree().sortItems(0, Qt::AscendingOrder);
}

void FileSystemSupportDialog::setupConnections()
{
    connect(dialogButtonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &FileSystemSupportDialog::close);
    connect(&dialogWidget().buttonRescan(), &QPushButton::clicked, this, &FileSystemSupportDialog::onButtonRescanClicked);
}

void FileSystemSupportDialog::onButtonRescanClicked()
{
    FileSystemFactory::init();
    setupDialog();
}
