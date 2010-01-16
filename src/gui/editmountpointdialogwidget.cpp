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
	MountEntry(const QString& n, const QString& p, const QString& t, const QStringList& o, qint32 d, qint32 pn) :
		name(n),
		path(p),
		type(t),
		options(o),
		dumpFreq(d),
		passNumber(pn)
	{
	}

	MountEntry(struct mntent* p) :
		name(p->mnt_fsname),
		path(p->mnt_dir),
		type(p->mnt_type),
		options(QString(p->mnt_opts).split(',')),
		dumpFreq(p->mnt_freq),
		passNumber(p->mnt_passno)
	{
	}

	QString name;
	QString path;
	QString type;
	QStringList options;
	qint32 dumpFreq;
	qint32 passNumber;
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
	QWidget(parent)
{
	readMountpoints("/etc/fstab");

	setupUi(this);

	labelName().setText(p.deviceNode());
	labelType().setText(p.fileSystem().name());

	if (mountPoints().find(p.deviceNode()) == mountPoints().end())
		mountPoints()[p.deviceNode()] = new MountEntry(p.deviceNode(), QString(), p.fileSystem().name(), QStringList(), 0, 0);

	MountEntry* entry = mountPoints()[p.deviceNode()];

	Q_ASSERT(entry);

	if (entry)
	{
		editPath().setText(entry->path);

		spinDumpFreq().setValue(entry->dumpFreq);
		spinPassNumber().setValue(entry->passNumber);

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

	struct mntent* p = NULL;

	while ((p = getmntent(fp)) != NULL)
	{
		QString device = p->mnt_fsname;

		if (device.startsWith("UUID="))
			device = findBlkIdDevice("UUID", QString(device).remove("UUID="));
		else if (device.startsWith("LABEL="))
			device = findBlkIdDevice("LABEL", QString(device).remove("LABEL="));

		if (!device.isEmpty())
		{
			QString mountPoint = p->mnt_dir;
			mountPoints()[device] = new MountEntry(p);
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
	MountEntry* mp = NULL;

	if (mountPoints().find(labelName().text()) == mountPoints().end())
	{
		kWarning() << "could not find device " << labelName().text() << " in mount points.";
		return false;
	}
	else
	{
		mp = mountPoints()[labelName().text()];

		mp->dumpFreq = spinDumpFreq().value();
		mp->passNumber = spinPassNumber().value();
		mp->path = editPath().text();
		mp->options = options();
	}

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
				i18nc("@title:window", "Error while saving mount points"));

	return rval;
}
