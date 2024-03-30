/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Abhijeet Sharma <sharma.abhijeet2096@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/editmountpointdialogwidget.h"
#include "gui/editmountoptionsdialog.h"

#include <core/partition.h>

#include <fs/filesystem.h>
#include <fs/luks.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QPointer>
#include <QString>

EditMountPointDialogWidget::EditMountPointDialogWidget(QWidget* parent, Partition& p) :
    QWidget(parent),
    m_Partition(p)
{
    m_fstabEntries = readFstabEntries();

    setupUi(this);

    m_deviceNode = partition().deviceNode();
    if (partition().roles().has(PartitionRole::Luks) && partition().fileSystem().type() != FileSystem::Type::Luks) {
        const FS::luks* luksFs = dynamic_cast<const FS::luks*>(&partition().fileSystem());
        m_deviceNode = luksFs->mapperName();
    }
    labelName().setText(m_deviceNode);
    labelType().setText(partition().fileSystem().name());

    bool entryFound = false;
    editPath().setEditable(true);
    for (auto &e : m_fstabEntries) {
        QString canonicalEntryPath = QFileInfo(e.deviceNode()).canonicalFilePath();
        QString canonicalDevicePath = QFileInfo(m_deviceNode).canonicalFilePath();
        if (canonicalEntryPath == canonicalDevicePath) {
            entryFound = true;
            entry.push_back(&e);
            mountPointList = possibleMountPoints(e.deviceNode());
        }
    }

    if (!entryFound) {
        FileSystem::Type type = partition().fileSystem().type();
        QString fsName;
        switch (type) {
        case FileSystem::Type::LinuxSwap:
            fsName = QStringLiteral("swap");
            break;
        case FileSystem::Type::Fat16:
        case FileSystem::Type::Fat32:
            fsName = QStringLiteral("vfat");
            break;
        default:
            fsName = partition().fileSystem().name({QStringLiteral("C")});
        }

        m_fstabEntries.push_back(FstabEntry(m_deviceNode, QString(), fsName, QString()));
        entry.push_back(&m_fstabEntries.back());
    }
    currentEntry = entry[0];
    editPath().addItems(mountPointList);
    spinDumpFreq().setValue(currentEntry->dumpFreq());
    spinPassNumber().setValue(currentEntry->passNumber());

    boxOptions()[QStringLiteral("ro")] = m_CheckReadOnly;
    boxOptions()[QStringLiteral("users")] = m_CheckUsers;
    boxOptions()[QStringLiteral("noauto")] = m_CheckNoAuto;
    boxOptions()[QStringLiteral("noatime")] = m_CheckNoAtime;
    boxOptions()[QStringLiteral("nodiratime")] = m_CheckNoDirAtime;
    boxOptions()[QStringLiteral("sync")] = m_CheckSync;
    boxOptions()[QStringLiteral("noexec")] = m_CheckNoExec;
    boxOptions()[QStringLiteral("relatime")] = m_CheckRelAtime;
    boxOptions()[QStringLiteral("nofail")] = m_CheckNoFail;

    setupRadio(currentEntry->entryType());
    setupOptions(currentEntry->options());

    connect(m_ButtonMore, &QPushButton::clicked, this, &EditMountPointDialogWidget::buttonMoreClicked);
    connect(m_ButtonSelect, &QPushButton::clicked, this, &EditMountPointDialogWidget::buttonSelectClicked);
    connect(m_EditPath, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index){ currentEntry = entry[index];
                        spinDumpFreq().setValue(currentEntry->dumpFreq());
                        spinPassNumber().setValue(currentEntry->passNumber());
                        setupRadio(currentEntry->entryType());
                        for (iterator_BoxOptions = boxOptions().begin(); iterator_BoxOptions != boxOptions().end(); ++iterator_BoxOptions){
                            boxOptions()[iterator_BoxOptions->first]->setChecked(false);
                        }
                        setupOptions(currentEntry->options());
                      });
    connect(m_EditPath, &QComboBox::currentTextChanged, this, &EditMountPointDialogWidget::currentPathChanged);
    currentPathChanged(m_EditPath->currentText());
}

EditMountPointDialogWidget::~EditMountPointDialogWidget()
{
}

bool EditMountPointDialogWidget::isValid() const
{
    return m_valid;
}

void EditMountPointDialogWidget::setupOptions(const QStringList& options)
{
    QStringList optTmpList;
    for (const auto &o : options) {
        if (boxOptions().find(o) != boxOptions().end())
            boxOptions()[o]->setChecked(true);
        else
            optTmpList.append(o);
    }

    m_Options = optTmpList.join(QLatin1Char(','));
}

void EditMountPointDialogWidget::setupRadio(const FstabEntry::Type entryType)
{
    if (partition().fileSystem().uuid().isEmpty()) {
        radioUUID().setEnabled(false);
        if (radioUUID().isChecked())
            radioDeviceNode().setChecked(true);
    }

    if (partition().fileSystem().label().isEmpty()) {
        radioLabel().setEnabled(false);
        if (radioLabel().isChecked())
            radioDeviceNode().setChecked(true);
    }
    switch (entryType) {
    case FstabEntry::Type::uuid:
        radioUUID().setChecked(true);
        break;

    case FstabEntry::Type::label:
        radioLabel().setChecked(true);
        break;

    case FstabEntry::Type::partuuid:
        radioUUID().setChecked(true);
        break;

    case FstabEntry::Type::partlabel:
        radioLabel().setChecked(true);
        break;

    case FstabEntry::Type::deviceNode:
        radioDeviceNode().setChecked(true);
        break;
    case FstabEntry::Type::comment:
        break;
    default:
        break;
    }
}

void EditMountPointDialogWidget::buttonSelectClicked(bool)
{
    const QString s = QFileDialog::getExistingDirectory(this, editPath().currentText());
    if (!s.isEmpty())
        editPath().setCurrentText(s);
}

void EditMountPointDialogWidget::removeMountPoint()
{
    for (auto it = fstabEntries().begin(); it != fstabEntries().end(); ++it) {
        if (editPath().count() <= 1 && (
                (it->fsSpec().contains(partition().deviceNode()) && !partition().deviceNode().isEmpty() ) ||
                (it->fsSpec().contains(partition().fileSystem().uuid()) && !partition().fileSystem().uuid().isEmpty() ) ||
                (it->fsSpec().contains(partition().fileSystem().label()) && !partition().fileSystem().label().isEmpty()) || 
                (it->fsSpec().contains(partition().label()) && !partition().label().isEmpty() ) ||
                (it->fsSpec().contains(partition().uuid()) && !partition().uuid().isEmpty() )))
        {
            fstabEntries().erase(it);
            partition().setMountPoint(QString());
        }
        else if (editPath().count() > 1 && (&*it == currentEntry))
        {
            fstabEntries().erase(it);
            editPath().removeItem(editPath().currentIndex());
            partition().setMountPoint(editPath().itemText(editPath().currentIndex()));
            break;
        }
    }

}

void EditMountPointDialogWidget::buttonMoreClicked(bool)
{
    QPointer<EditMountOptionsDialog>  dlg = new EditMountOptionsDialog(this, m_Options.split(QLatin1Char(',')));

    if (dlg->exec() == QDialog::Accepted)
        setupOptions(dlg->options());

    delete dlg;
}

QStringList EditMountPointDialogWidget::options() const
{
    QStringList optList = m_Options.split(QLatin1Char(','), Qt::SkipEmptyParts);

    const auto keys = boxOptions();
    for (const auto &s : keys)
        if (s.second->isChecked())
            optList.append(s.first);

    return optList;
}

void EditMountPointDialogWidget::acceptChanges()
{
    currentEntry->setDumpFreq(spinDumpFreq().value());
    currentEntry->setPassNumber(spinPassNumber().value());
    currentEntry->setMountPoint(editPath().currentText());
    currentEntry->setOptions(options());

    if (radioUUID().isChecked() && !partition().fileSystem().uuid().isEmpty())
        currentEntry->setFsSpec(QStringLiteral("UUID=") + partition().fileSystem().uuid());
    else if (radioLabel().isChecked() && !partition().fileSystem().label().isEmpty())
        currentEntry->setFsSpec(QStringLiteral("LABEL=") + partition().fileSystem().label());
    else
        currentEntry->setFsSpec(m_deviceNode);
}

void EditMountPointDialogWidget::currentPathChanged(const QString &newPath)
{
    auto path = newPath.trimmed().toLower();
    if (path.isEmpty() || path == QStringLiteral("none")) {
        m_valid = false;
        m_PathMessage->setVisible(true);
    } else {
        m_valid = true;
        m_PathMessage->setVisible(false);
    }
    Q_EMIT isValidChanged(m_valid);
}
