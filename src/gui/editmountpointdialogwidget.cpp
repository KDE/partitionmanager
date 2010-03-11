/***************************************************************************
 *   Copyright (C) 2009,2010 by Volker Lanz <vl@fidra.de>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "gui/editmountpointdialogwidget.h"
#include "gui/editmountoptionsdialog.h"

#include "core/partition.h"

#include "fs/filesystem.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kmountpoint.h>
#include <kmessagebox.h>

#include <QString>
#include <QWidget>
#include <QFile>
#include <QPointer>

#include <mntent.h>
#include <blkid/blkid.h>

struct MountEntry
{
	enum IdentifyType { deviceNode, uuid, label };

	MountEntry(const QString& n, const QString& p, const QString& t, const QStringList& o, qint32 d, qint32 pn, IdentifyType type) :
		name(n),
		path(p),
		type(t),
		options(o),
		dumpFreq(d),
		passNumber(pn),
		identifyType(type)
	{
	}

	MountEntry(struct mntent* p, IdentifyType type) :
		name(p->mnt_fsname),
		path(p->mnt_dir),
		type(p->mnt_type),
		options(QString(p->mnt_opts).split(',')),
		dumpFreq(p->mnt_freq),
		passNumber(p->mnt_passno),
		identifyType(type)
	{
	}

	QString name;
	QString path;
	QString type;
	QStringList options;
	qint32 dumpFreq;
	qint32 passNumber;
	IdentifyType identifyType;
};

static QString findBlkIdDevice(const QString& token, const QString& value)
{
	blkid_cache cache;
	QString rval;

	if (blkid_get_cache(&cache, NULL) == 0)
	{
		if (char* c = blkid_evaluate_tag(token.toLocal8Bit(), value.toLocal8Bit(), &cache))
		{
			rval = c;
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
	readMountpoints("/etc/fstab");

	setupUi(this);

	labelName().setText(partition().deviceNode());
	labelType().setText(partition().fileSystem().name());

	if (mountPoints().find(partition().deviceNode()) == mountPoints().end())
		mountPoints()[partition().deviceNode()] = new MountEntry(partition().deviceNode(), QString(), partition().fileSystem().name(), QStringList(), 0, 0, MountEntry::deviceNode);

	MountEntry* entry = mountPoints()[partition().deviceNode()];

	Q_ASSERT(entry);

	if (entry)
	{
		editPath().setText(entry->path);

		spinDumpFreq().setValue(entry->dumpFreq);
		spinPassNumber().setValue(entry->passNumber);

		switch(entry->identifyType)
		{
			case MountEntry::uuid:
				radioUUID().setChecked(true);
				break;

			case MountEntry::label:
				radioLabel().setChecked(true);
				break;

			default:
				radioDeviceNode().setChecked(true);
		}

		boxOptions()["ro"] = m_CheckReadOnly;
		boxOptions()["users"] = m_CheckUsers;
		boxOptions()["noauto"] = m_CheckNoAuto;
		boxOptions()["noatime"] = m_CheckNoAtime;
		boxOptions()["nodiratime"] = m_CheckNoDirAtime;
		boxOptions()["sync"] = m_CheckSync;
		boxOptions()["noexec"] = m_CheckNoExec;
		boxOptions()["relatime"] = m_CheckRelAtime;

		setupOptions(entry->options);
	}

	if (partition().fileSystem().uuid().isEmpty())
	{
		radioUUID().setEnabled(false);
		if (radioUUID().isChecked())
			radioDeviceNode().setChecked(true);
	}

	if (partition().fileSystem().label().isEmpty())
	{
		radioLabel().setEnabled(false);
		if (radioLabel().isChecked())
			radioDeviceNode().setChecked(true);
	}
}

EditMountPointDialogWidget::~EditMountPointDialogWidget()
{
	qDeleteAll(mountPoints().values());
}

void EditMountPointDialogWidget::setupOptions(const QStringList& options)
{
	QStringList optTmpList;

	foreach (const QString& o, options)
		if (boxOptions().find(o) != boxOptions().end())
			boxOptions()[o]->setChecked(true);
		else
			optTmpList.append(o);

	m_Options = optTmpList.join(",");
}

void EditMountPointDialogWidget::on_m_ButtonSelect_clicked(bool)
{
	const QString s = KFileDialog::getExistingDirectory(KUrl(editPath().text()), this);
	if (!s.isEmpty())
		editPath().setText(s);
}

void EditMountPointDialogWidget::on_m_ButtonMore_clicked(bool)
{
	QPointer<EditMountOptionsDialog>  dlg = new EditMountOptionsDialog(this, m_Options.split(','));

	if (dlg->exec() == KDialog::Accepted)
		setupOptions(dlg->options());

	delete dlg;
}

QStringList EditMountPointDialogWidget::options()
{
	QStringList optList = m_Options.split(',', QString::SkipEmptyParts);

	foreach (const QString& s, boxOptions().keys())
		if (boxOptions()[s]->isChecked())
			optList.append(s);

	return optList;
}

bool EditMountPointDialogWidget::readMountpoints(const QString& filename)
{
	FILE* fp = setmntent(filename.toLocal8Bit(), "r");

	if (fp == NULL)
	{
		KMessageBox::sorry(this,
				i18nc("@info", "Could not open mount point file <filename>%1</filename>.", filename),
				i18nc("@title:window", "Error while reading mount points"));
		return false;
	}

	struct mntent* mnt = NULL;

	while ((mnt = getmntent(fp)) != NULL)
	{
		QString device = mnt->mnt_fsname;
		MountEntry::IdentifyType type = MountEntry::deviceNode;

		if (device.startsWith("UUID="))
		{
			type = MountEntry::uuid;
			device = findBlkIdDevice("UUID", QString(device).remove("UUID="));
		}
		else if (device.startsWith("LABEL="))
		{
			type = MountEntry::label;
			device = findBlkIdDevice("LABEL", QString(device).remove("LABEL="));
		}

		if (!device.isEmpty())
		{
			QString mountPoint = mnt->mnt_dir;
			mountPoints()[device] = new MountEntry(mnt, type);
		}
	}

	endmntent(fp);

	return true;
}

static void writeEntry(QFile& output, const MountEntry* entry)
{
	Q_ASSERT(entry);

	if (entry == NULL)
		return;

	if (entry->path.isEmpty())
		return;

	QTextStream s(&output);

	s << entry->name << "\t"
		<< entry->path << "\t"
		<< entry->type << "\t"
		<< (entry->options.size() > 0 ? entry->options.join(",") : "defaults") << "\t"
		<< entry->dumpFreq << "\t"
		<< entry->passNumber << "\n";
}

bool EditMountPointDialogWidget::acceptChanges()
{
	MountEntry* entry = NULL;

	if (mountPoints().find(labelName().text()) == mountPoints().end())
	{
		kWarning() << "could not find device " << labelName().text() << " in mount points.";
		return false;
	}

	entry = mountPoints()[labelName().text()];

	entry->dumpFreq = spinDumpFreq().value();
	entry->passNumber = spinPassNumber().value();
	entry->path = editPath().text();
	entry->options = options();

	if (radioUUID().isChecked() && !partition().fileSystem().uuid().isEmpty())
		entry->name = "UUID=" + partition().fileSystem().uuid();
	else if (radioLabel().isChecked() && !partition().fileSystem().label().isEmpty())
		entry->name = "LABEL=" + partition().fileSystem().label();
	else
		entry->name = partition().deviceNode();

	return true;
}

bool EditMountPointDialogWidget::writeMountpoints(const QString& filename)
{
	bool rval = true;
	const QString newFilename = QString("%1.new").arg(filename);
	QFile out(newFilename);

	if (!out.open(QFile::ReadWrite | QFile::Truncate))
	{
		kWarning() << "could not open output file " << newFilename;
		rval = false;
	}
	else
	{
		foreach (const MountEntry* me, mountPoints())
			writeEntry(out, me);

		out.close();

		const QString bakFilename = QString("%1.bak").arg(filename);
		QFile::remove(bakFilename);

		if (!QFile::rename(filename, bakFilename))
		{
			kWarning() << "could not rename " << filename << " to " << bakFilename;
			rval = false;
		}

		if (rval && !QFile::rename(newFilename, filename))
		{
			kWarning() << "could not rename " << newFilename << " to " << filename;
			rval = false;
		}
	}

	if (!rval)
		KMessageBox::sorry(this,
				i18nc("@info", "Could not save mount points to file <filename>%1</filename>.", filename),
				i18nc("@title:window", "Error While Saving Mount Points"));

	return rval;
}
