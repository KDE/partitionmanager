/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "gui/partpropsdialog.h"
#include "gui/partpropswidget.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystemfactory.h"

#include "util/capacity.h"
#include "util/helpers.h"

#include <kmessagebox.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <klineedit.h>

#include <QtAlgorithms>

/** Creates a new PartPropsDialog
	@param parent pointer to the parent widget
	@param d the Device the Partition is on
	@param p the Partition to show properties for
*/
PartPropsDialog::PartPropsDialog(QWidget* parent, Device& d, Partition& p) :
	KDialog(parent),
	m_Device(d),
	m_Partition(p),
	m_WarnFileSystemChange(false),
	m_DialogWidget(new PartPropsWidget(this)),
	m_ReadOnly(partition().isMounted() || partition().state() == Partition::StateCopy || partition().state() == Partition::StateRestore || d.partitionTable()->isReadOnly()),
	m_ForceRecreate(false)
{
	setMainWidget(&dialogWidget());
	setCaption(i18nc("@title:window", "Partition properties: <filename>%1</filename>", partition().deviceNode()));

	setupDialog();
	setupConnections();

	restoreDialogSize(KConfigGroup(KGlobal::config(), "partPropsDialog"));
}

/** Destroys a PartPropsDialog */
PartPropsDialog::~PartPropsDialog()
{
	KConfigGroup kcg(KGlobal::config(), "partPropsDialog");
	saveDialogSize(kcg);
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
	setDefaultButton(KDialog::Cancel);
	enableButtonOk(false);
	button(KDialog::Cancel)->setFocus();

	dialogWidget().partWidget().init(&partition());

	const QString mp = partition().mountPoint().isEmpty()
			? i18nc("@item mountpoint", "(none found)")
			: partition().mountPoint();
	dialogWidget().mountPoint().setText(mp);

	dialogWidget().role().setText(partition().roles().toString());

	QString statusText = i18nc("@label partition state", "idle");
	if (partition().isMounted())
	{
		if (partition().roles().has(PartitionRole::Extended))
			statusText = i18nc("@label partition state", "At least one logical partition is mounted.");
		else if (!partition().mountPoint().isEmpty())
			statusText = i18nc("@label partition state", "mounted on <filename>%1</filename>", mp);
		else
			statusText = i18nc("@label partition state", "mounted");
	}

	dialogWidget().status().setText(statusText);
	dialogWidget().uuid().setText(partition().fileSystem().uuid().isEmpty() ? i18nc("@item uuid", "(none)") : partition().fileSystem().uuid());

	setupFileSystemComboBox();

	 // don't do this before the file system combo box has been set up!
	dialogWidget().label().setText(newLabel().isEmpty() ? partition().fileSystem().label() : newLabel());
	dialogWidget().capacity().setText(Capacity(partition()).toString(Capacity::AppendUnit | Capacity::AppendBytes));

	if (Capacity(partition(), Capacity::Available).isValid())
	{
		const qint64 availPercent = (partition().fileSystem().length() - partition().fileSystem().sectorsUsed()) * 100 / partition().fileSystem().length();

		const QString availString = QString("%1% - %2")
			.arg(availPercent)
			.arg(Capacity(partition(), Capacity::Available).toString(Capacity::AppendUnit | Capacity::AppendBytes));
		const QString usedString = QString("%1% - %2")
			.arg(100 - availPercent)
			.arg(Capacity(partition(), Capacity::Used).toString(Capacity::AppendUnit | Capacity::AppendBytes));

		dialogWidget().available().setText(availString);
		dialogWidget().used().setText(usedString);
	}
	else
	{
		dialogWidget().available().setText(Capacity::invalidString());
		dialogWidget().used().setText(Capacity::invalidString());
	}

	dialogWidget().firstSector().setText(KGlobal::locale()->formatNumber(partition().firstSector(), 0));
	dialogWidget().lastSector().setText(KGlobal::locale()->formatNumber(partition().lastSector(), 0));
	dialogWidget().numSectors().setText(KGlobal::locale()->formatNumber(partition().length(), 0));

	setupFlagsList();

	updateHideAndShow();

	setMinimumSize(dialogWidget().size());
	resize(dialogWidget().size());
}

void PartPropsDialog::setupFlagsList()
{
	int f = 1;
	QString s;
	while(!(s = PartitionTable::flagName(static_cast<PartitionTable::Flag>(f))).isEmpty())
	{
		if (partition().availableFlags() & f)
		{
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
	const FileSystem* fs = FileSystemFactory::create(newFileSystemType(), -1, -1, -1, "");

	if (fs == NULL || fs->supportSetLabel() == FileSystem::cmdSupportNone)
	{
		dialogWidget().label().setReadOnly(true);
		dialogWidget().noSetLabel().setVisible(true);
		dialogWidget().noSetLabel().setFont(KGlobalSettings::smallestReadableFont());

		QPalette palette = dialogWidget().noSetLabel().palette();
		QColor f = palette.color(QPalette::Foreground);
		f.setAlpha(128);
		palette.setColor(QPalette::Foreground, f);
		dialogWidget().noSetLabel().setPalette(palette);
	}
	else
	{
		dialogWidget().label().setReadOnly(isReadOnly());
		dialogWidget().noSetLabel().setVisible(false);
	}

	// when do we show the uuid?
	const bool showUuid =
			partition().state() != Partition::StateNew &&                           // not for new partitions
			!(fs == NULL || fs->supportGetUUID() == FileSystem::cmdSupportNone);       // not if the FS doesn't support it

	dialogWidget().showUuid(showUuid);

	delete fs;

	// when do we show available and used capacity?
	const bool showAvailableAndUsed =
			partition().state() != Partition::StateNew &&                           // not for new partitions
			!partition().roles().has(PartitionRole::Extended) &&                    // neither for extended
			!partition().roles().has(PartitionRole::Unallocated) &&                 // or for unallocated
			newFileSystemType() != FileSystem::Unformatted;                         // and not for unformatted file systems

	dialogWidget().showAvailable(showAvailableAndUsed);
	dialogWidget().showUsed(showAvailableAndUsed);

	// when do we show the file system combo box?
	const bool showFileSystem =
			!partition().roles().has(PartitionRole::Extended) &&                    // not for extended, they have no file system
			!partition().roles().has(PartitionRole::Unallocated);                   // and not for unallocated: no choice there

	dialogWidget().showFileSystem(showFileSystem);

	// when do we show the recreate file system check box?
	const bool showCheckRecreate =
			showFileSystem &&                                                       // only if we also show the file system
			partition().fileSystem().supportCreate() != FileSystem::cmdSupportNone &&  // and support creating this file system
			partition().fileSystem().type() != FileSystem::Unknown &&               // and not for unknown file systems
			partition().state() != Partition::StateNew;                             // or new partitions

	dialogWidget().showCheckRecreate(showCheckRecreate);

	// when do we show the list of partition flags?
	const bool showListFlags =
			partition().state() != Partition::StateNew &&                           // not for new partitions
			!partition().roles().has(PartitionRole::Unallocated);                   // and not for unallocated space

	dialogWidget().showListFlags(showListFlags);

	dialogWidget().checkRecreate().setEnabled(!isReadOnly());
	dialogWidget().listFlags().setEnabled(!isReadOnly());
	dialogWidget().fileSystem().setEnabled(!isReadOnly() && !forceRecreate());
}

void PartPropsDialog::setupConnections()
{
	connect(&dialogWidget().label(), SIGNAL(textEdited(const QString&)), SLOT(setDirty()));
	connect(&dialogWidget().fileSystem(), SIGNAL(currentIndexChanged(int)), SLOT(onFilesystemChanged(int)));
	connect(&dialogWidget().checkRecreate(), SIGNAL(stateChanged(int)), SLOT(onRecreate(int)));

	// We want to enable the OK-button whenever the user checks or unchecks a flag in the flag list.
	// But it seems Qt doesn't offer a foolproof way to detect if this has happened: The currentRow/ItemChanged
	// signal only means the _current_ item has changed, but not necessarily that it was checked/unchecked. And
	// itemClicked alone isn't enough either. We choose to rather enable the OK-button too often than too
	// seldom.
	connect(&dialogWidget().listFlags(), SIGNAL(itemClicked(QListWidgetItem*)), SLOT(setDirty()));
	connect(&dialogWidget().listFlags(), SIGNAL(currentRowChanged(int)), SLOT(setDirty()));

}

void PartPropsDialog::setDirty()
{
	setDefaultButton(KDialog::Ok);
	enableButtonOk(true);
}

void PartPropsDialog::setupFileSystemComboBox()
{
	dialogWidget().fileSystem().clear();
	QString selected;
	QStringList fsNames;

	foreach(const FileSystem* fs, FileSystemFactory::map())
		if (partition().fileSystem().type() == fs->type() || (fs->supportCreate() != FileSystem::cmdSupportNone && partition().capacity() >= fs->minCapacity() && partition().capacity() <= fs->maxCapacity()))
		{
			QString name = fs->name();

			if (partition().fileSystem().type() == fs->type())
				selected = name;

			// If the partition isn't extended, skip the extended FS
			if (fs->type() == FileSystem::Extended && !partition().roles().has(PartitionRole::Extended))
				continue;

			// The user cannot change the filesystem back to "unformatted" once a filesystem has been created.
			if (fs->type() == FileSystem::Unformatted)
			{
				// .. but if the file system is unknown to us, show the unformatted option as the currently selected one
				if (partition().fileSystem().type() == FileSystem::Unknown)
				{
					name = FileSystem::nameForType(FileSystem::Unformatted);
					selected = name;
				}
				else if (partition().fileSystem().type() != FileSystem::Unformatted && partition().state() != Partition::StateNew)
					continue;
			}

			fsNames.append(name);
		}

	qSort(fsNames.begin(), fsNames.end(), caseInsensitiveLessThan);

	foreach (const QString& fsName, fsNames)
		dialogWidget().fileSystem().addItem(createFileSystemColor(FileSystem::typeForName(fsName), 8), fsName);

	dialogWidget().fileSystem().setCurrentIndex(dialogWidget().fileSystem().findText(selected));
}

void PartPropsDialog::updatePartitionFileSystem()
{
	FileSystem* fs = FileSystemFactory::create(newFileSystemType(), partition().firstSector(), partition().lastSector());
	partition().deleteFileSystem();
	partition().setFileSystem(fs);
	dialogWidget().partWidget().update();
}

void PartPropsDialog::onFilesystemChanged(int)
{
	if (partition().state() == Partition::StateNew || warnFileSystemChange() || KMessageBox::warningContinueCancel(this,
			i18nc("@info", "<para><warning>You are about to lose all data on partition <filename>%1</filename>.</warning></para>"
				"<para>Changing the file system on a partition already on disk will erase all its contents. If you continue now and apply the resulting operation in the main window, all data on <filename>%1</filename> will unrecoverably be lost.</para>", partition().deviceNode()),
			i18nc("@title:window", "Really Recreate <filename>%1</filename> with File System %2?", partition().deviceNode(), dialogWidget().fileSystem().currentText()),
			KGuiItem(i18nc("@action:button", "Change the File System"), "arrow-right"),
			KGuiItem(i18nc("@action:button", "Do Not Change the File System"), "dialog-cancel"), "reallyChangeFileSystem") == KMessageBox::Continue)
	{
		setDirty();
		updateHideAndShow();
		setWarnFileSystemChange();
		updatePartitionFileSystem();
	}
	else
	{
		dialogWidget().fileSystem().disconnect(this);
		setupFileSystemComboBox();
		connect(&dialogWidget().fileSystem(), SIGNAL(currentIndexChanged(int)), SLOT(onFilesystemChanged(int)));
	}
}

void PartPropsDialog::onRecreate(int state)
{
	if (state == Qt::Checked && (warnFileSystemChange() || KMessageBox::warningContinueCancel(this,
			i18nc("@info", "<para><warning>You are about to lose all data on partition <filename>%1</filename>.</warning></para>"
				"<para>Recreating a file system will erase all its contents. If you continue now and apply the resulting operation in the main window, all data on <filename>%1</filename> will unrecoverably be lost.</para>", partition().deviceNode()),
			i18nc("@title:window", "Really Recreate File System on <filename>%1</filename>?", partition().deviceNode()),
			KGuiItem(i18nc("@action:button", "Recreate the File System"), "arrow-right"),
			KGuiItem(i18nc("@action:button", "Do Not Recreate the File System"), "dialog-cancel"), "reallyRecreateFileSystem") == KMessageBox::Continue))
	{
		setDirty();
		setWarnFileSystemChange();
		setForceRecreate(true);
		dialogWidget().fileSystem().setCurrentIndex(dialogWidget().fileSystem().findText(partition().fileSystem().name()));
		dialogWidget().fileSystem().setEnabled(false);
		updateHideAndShow();
		updatePartitionFileSystem();
	}
	else
	{
		setForceRecreate(false);
		dialogWidget().checkRecreate().setCheckState(Qt::Unchecked);
		dialogWidget().fileSystem().setEnabled(true);
		updateHideAndShow();
	}
}
