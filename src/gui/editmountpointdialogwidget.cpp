/*************************************************************************
 *  Copyright (C) 2009, 2010 by Volker Lanz <vl@fidra.de>                *
 *  Copyright (C) 2016, 2017 by Andrius Štikonas <andrius@stikonas.eu>   *
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
    if (partition().roles().has(PartitionRole::Luks) && partition().fileSystem().type() != FileSystem::Luks) {
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
        if (canonicalEntryPath == canonicalDevicePath) { // FIXME fix multiple mountpoints
            entryFound = true;
            entry.append(&e);
            mountPointList = possibleMountPoints(e.deviceNode());
        }
    }

    if (!entryFound) {
        FileSystem::Type type = partition().fileSystem().type();
        QString fsName;
        switch (type) {
        case FileSystem::LinuxSwap:
            fsName = QStringLiteral("swap");
            break;
        case FileSystem::Fat16:
        case FileSystem::Fat32:
            fsName = QStringLiteral("vfat");
            break;
        default:
            fsName = partition().fileSystem().name();
        }

        m_fstabEntries.append(FstabEntry(m_deviceNode, QString(), fsName, QString()));
        entry.append(&m_fstabEntries.last());
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
}

EditMountPointDialogWidget::~EditMountPointDialogWidget()
{
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

void EditMountPointDialogWidget::setupRadio(const FstabEntryType entryType)
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
    case FstabEntryType::uuid:
        radioUUID().setChecked(true);
        break;

    case FstabEntryType::label:
        radioLabel().setChecked(true);
        break;

    case FstabEntryType::partuuid:
        radioUUID().setChecked(true);
        break;

    case FstabEntryType::partlabel:
        radioLabel().setChecked(true);
        break;

    case FstabEntryType::deviceNode:
        radioDeviceNode().setChecked(true);
        break;
    case FstabEntryType::comment:
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
    int i=0;
    for (const auto &e : fstabEntries()) {
       if(editPath().count()<=1 && ((e.fsSpec().contains(partition().deviceNode()) && !partition().deviceNode().isEmpty() ) || (e.fsSpec().contains(partition().fileSystem().uuid()) && !partition().fileSystem().uuid().isEmpty()) ||
           (e.fsSpec().contains(partition().fileSystem().label()) && !partition().fileSystem().label().isEmpty()) || (e.fsSpec().contains(partition().label()) && !partition().label().isEmpty() ) || (e.fsSpec().contains(partition().uuid()) && !partition().uuid().isEmpty()  )))
       {
            fstabEntries().removeAt(i);
            partition().setMountPoint(QString());
            i--;
       }
       else if(editPath().count()>1 && ((&e == currentEntry)))
       {
            fstabEntries().removeAt(i);
            editPath().removeItem(editPath().currentIndex());
            partition().setMountPoint(editPath().itemText(editPath().currentIndex()));
            i--;
            break;
        }
        i++;
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
    QStringList optList = m_Options.split(QLatin1Char(','), QString::SkipEmptyParts);

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
