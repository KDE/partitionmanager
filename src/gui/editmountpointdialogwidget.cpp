/*************************************************************************
 *  Copyright (C) 2009, 2010 by Volker Lanz <vl@fidra.de>                *
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

#include "gui/editmountpointdialogwidget.h"
#include "gui/editmountoptionsdialog.h"

#include <core/partition.h>
#include <core/mountentry.h>

#include <fs/filesystem.h>
#include <fs/luks.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <KMountPoint>

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QPointer>
#include <QString>

#include <mntent.h>
#include <blkid/blkid.h>

static QString findBlkIdDevice(const QString& token, const QString& value)
{
    blkid_cache cache;
    QString rval;

    if (blkid_get_cache(&cache, nullptr) == 0) {
        if (char* c = blkid_evaluate_tag(token.toLocal8Bit().constData(), value.toLocal8Bit().constData(), &cache)) {
            rval = QString::fromLocal8Bit(c);
            free(c);
        }

        blkid_put_cache(cache);
    }

    return rval;
}

EditMountPointDialogWidget::EditMountPointDialogWidget(QWidget* parent, const Partition& p) :
    QWidget(parent),
    m_Partition(p)
{
    readMountpoints(QStringLiteral("/etc/fstab"));

    setupUi(this);

    m_deviceNode = partition().deviceNode();
    if (partition().roles().has(PartitionRole::Luks) && partition().fileSystem().type() != FileSystem::Luks) {
        const FS::luks* luksFs = dynamic_cast<const FS::luks*>(&partition().fileSystem());
        m_deviceNode = luksFs->mapperName();
    }
    labelName().setText(m_deviceNode);
    labelType().setText(partition().fileSystem().name());

    if (mountPoints().find(m_deviceNode) == mountPoints().end())
        mountPoints().insert(std::pair<QString, MountEntry*>(m_deviceNode, new MountEntry(m_deviceNode, QString(), partition().fileSystem().name(), QStringList(), 0, 0, MountEntry::deviceNode)));

    auto search = mountPoints().find(m_deviceNode); // FIXME: Only one mountpoint entry corresponding to given device is shown
    MountEntry* entry = search->second;

    Q_ASSERT(entry);

    if (entry) {
        editPath().setText(entry->path);

        spinDumpFreq().setValue(entry->dumpFreq);
        spinPassNumber().setValue(entry->passNumber);

        switch (entry->identifyType) {
        case MountEntry::uuid:
            radioUUID().setChecked(true);
            break;

        case MountEntry::label:
            radioLabel().setChecked(true);
            break;

        default:
            radioDeviceNode().setChecked(true);
        }

        boxOptions()[QStringLiteral("ro")] = m_CheckReadOnly;
        boxOptions()[QStringLiteral("users")] = m_CheckUsers;
        boxOptions()[QStringLiteral("noauto")] = m_CheckNoAuto;
        boxOptions()[QStringLiteral("noatime")] = m_CheckNoAtime;
        boxOptions()[QStringLiteral("nodiratime")] = m_CheckNoDirAtime;
        boxOptions()[QStringLiteral("sync")] = m_CheckSync;
        boxOptions()[QStringLiteral("noexec")] = m_CheckNoExec;
        boxOptions()[QStringLiteral("relatime")] = m_CheckRelAtime;

        setupOptions(entry->options);
    }

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

    connect(m_ButtonMore, &QPushButton::clicked, this, &EditMountPointDialogWidget::buttonMoreClicked);
    connect(m_ButtonSelect, &QPushButton::clicked, this, &EditMountPointDialogWidget::buttonSelectClicked);
}

EditMountPointDialogWidget::~EditMountPointDialogWidget()
{
    for (const auto &mp : mountPoints())
        delete mp.second;
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

    m_Options = optTmpList.join(QStringLiteral(","));
}

void EditMountPointDialogWidget::buttonSelectClicked(bool)
{
    const QString s = QFileDialog::getExistingDirectory(this, editPath().text());
    if (!s.isEmpty())
        editPath().setText(s);
}

void EditMountPointDialogWidget::buttonMoreClicked(bool)
{
    QPointer<EditMountOptionsDialog>  dlg = new EditMountOptionsDialog(this, m_Options.split(QStringLiteral(",")));

    if (dlg->exec() == QDialog::Accepted)
        setupOptions(dlg->options());

    delete dlg;
}

QStringList EditMountPointDialogWidget::options() const
{
    QStringList optList = m_Options.split(QStringLiteral(","), QString::SkipEmptyParts);

    const auto keys = boxOptions();
    for (const auto &s : keys)
        if (s.second->isChecked())
            optList.append(s.first);

    return optList;
}

bool EditMountPointDialogWidget::readMountpoints(const QString& filename)
{
    FILE* fp = setmntent(filename.toLocal8Bit().constData(), "r");

    if (fp == nullptr) {
        KMessageBox::sorry(this,
                           xi18nc("@info", "Could not open mount point file <filename>%1</filename>.", filename),
                           xi18nc("@title:window", "Error while reading mount points"));
        return false;
    }

    struct mntent* mnt = nullptr;

    while ((mnt = getmntent(fp)) != nullptr) {
        QString device = QString::fromUtf8(mnt->mnt_fsname);
        MountEntry::IdentifyType type = MountEntry::deviceNode;

        if (device.startsWith(QStringLiteral("UUID="))) {
            type = MountEntry::uuid;
            device = findBlkIdDevice(QStringLiteral("UUID"), QString(device).remove(QStringLiteral("UUID=")));
        } else if (device.startsWith(QStringLiteral("LABEL="))) {
            type = MountEntry::label;
            device = findBlkIdDevice(QStringLiteral("LABEL"), QString(device).remove(QStringLiteral("LABEL=")));
        } else if (device.startsWith(QStringLiteral("/")))
            device = QFile::symLinkTarget(device);

        if (!device.isEmpty()) {
            QString mountPoint = QString::fromUtf8(mnt->mnt_dir);
            mountPoints().insert(std::pair<QString, MountEntry*>(device, new MountEntry(mnt, type)));
        }
    }

    endmntent(fp);

    return true;
}

static void writeEntry(QFile& output, const MountEntry* entry)
{
    Q_ASSERT(entry);

    if (entry == nullptr)
        return;

    if (entry->path.isEmpty())
        return;

    QTextStream s(&output);

    s << entry->name << "\t"
      << entry->path << "\t"
      << entry->type << "\t"
      << (entry->options.size() > 0 ? entry->options.join(QStringLiteral(",")) : QStringLiteral("defaults")) << "\t"
      << entry->dumpFreq << "\t"
      << entry->passNumber << "\n";
}

bool EditMountPointDialogWidget::acceptChanges()
{
    MountEntry* entry = nullptr;

    if (mountPoints().find(labelName().text()) == mountPoints().end()) {
        qWarning() << "could not find device " << labelName().text() << " in mount points.";
        return false;
    }

    auto search = mountPoints().find(labelName().text());
    entry = search->second;

    entry->dumpFreq = spinDumpFreq().value();
    entry->passNumber = spinPassNumber().value();
    entry->path = editPath().text();
    entry->options = options();

    if (radioUUID().isChecked() && !partition().fileSystem().uuid().isEmpty())
        entry->name = QStringLiteral("UUID=") + partition().fileSystem().uuid();
    else if (radioLabel().isChecked() && !partition().fileSystem().label().isEmpty())
        entry->name = QStringLiteral("LABEL=") + partition().fileSystem().label();
    else
        entry->name = m_deviceNode;

    return true;
}

bool EditMountPointDialogWidget::writeMountpoints(const QString& filename)
{
    bool rval = true;
    const QString newFilename = QStringLiteral("%1.new").arg(filename);
    QFile out(newFilename);

    if (!out.open(QFile::ReadWrite | QFile::Truncate)) {
        qWarning() << "could not open output file " << newFilename;
        rval = false;
    } else {
        const auto mp = mountPoints();
        for (const auto &me : mp)
            writeEntry(out, me.second);

        out.close();

        const QString bakFilename = QStringLiteral("%1.bak").arg(filename);
        QFile::remove(bakFilename);

        if (QFile::exists(filename) && !QFile::rename(filename, bakFilename)) {
            qWarning() << "could not rename " << filename << " to " << bakFilename;
            rval = false;
        }

        if (rval && !QFile::rename(newFilename, filename)) {
            qWarning() << "could not rename " << newFilename << " to " << filename;
            rval = false;
        }
    }

    if (!rval)
        KMessageBox::sorry(this,
                           xi18nc("@info", "Could not save mount points to file <filename>%1</filename>.", filename),
                           xi18nc("@title:window", "Error While Saving Mount Points"));

    return rval;
}
