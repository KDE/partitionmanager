/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
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

#include "gui/newdialog.h"
#include "gui/sizedialogwidget.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/capacity.h"

#include <kdebug.h>

#include <QtAlgorithms>

/** Creates a NewDialog
	@param parent the parent widget
	@param device the Device on which a new Partition is to be created
	@param unallocatedPartition the unallocated space on the Device to create a Partition in
	@param r the permitted Roles for the new Partition
*/
NewDialog::NewDialog(QWidget* parent, Device& device, Partition& unallocatedPartition, PartitionRole::Roles r) :
	SizeDialogBase(parent, Capacity::MiB, device, unallocatedPartition, 0, 0),
	m_PartitionRoles(r)
{
	setMainWidget(&dialogWidget());
	setCaption(i18nc("@title:window", "Create a new partition"));

	setupDialog();
	setupConstraints();
	setupConnections();

	restoreDialogSize(KConfigGroup(KGlobal::config(), "newDialog"));
}

NewDialog::~NewDialog()
{
	KConfigGroup kcg(KGlobal::config(), "newDialog");
	saveDialogSize(kcg);
}

void NewDialog::setupDialog()
{
	QStringList fsNames;
	foreach (const FileSystem* fs, FileSystemFactory::map().values())
		if (fs->supportCreate() != FileSystem::SupportNone && fs->type() != FileSystem::Extended)
			fsNames.append(fs->name());

	qSort(fsNames);
	dialogWidget().comboFileSystem().addItems(fsNames);

	dialogWidget().radioPrimary().setVisible(partitionRoles() & PartitionRole::Primary);
	dialogWidget().radioExtended().setVisible(partitionRoles() & PartitionRole::Extended);
	dialogWidget().radioLogical().setVisible(partitionRoles() & PartitionRole::Logical);

	if (partitionRoles() & PartitionRole::Primary)
		dialogWidget().radioPrimary().setChecked(true);
	else
		dialogWidget().radioLogical().setChecked(true);

	SizeDialogBase::setupDialog();

	// don't move these above the call to parent's setupDialog, because only after that has
	// run there is a valid partition set in the part resizer widget and they will need that.
	onRoleChanged(false);
	onFilesystemChanged(0);
}

void NewDialog::setupConnections()
{
	connect(&dialogWidget().radioPrimary(), SIGNAL(toggled(bool)), SLOT(onRoleChanged(bool)));
	connect(&dialogWidget().radioExtended(), SIGNAL(toggled(bool)), SLOT(onRoleChanged(bool)));
	connect(&dialogWidget().radioLogical(), SIGNAL(toggled(bool)), SLOT(onRoleChanged(bool)));
	connect(&dialogWidget().comboFileSystem(), SIGNAL(currentIndexChanged(int)), SLOT(onFilesystemChanged(int)));
	connect(&dialogWidget().label(), SIGNAL(textChanged(const QString&)), SLOT(onLabelChanged(const QString&)));

	SizeDialogBase::setupConnections();
}

void NewDialog::accept()
{
	if (partition().roles().has(PartitionRole::Extended))
	{
		partition().deleteFileSystem();
		partition().setFileSystem(FileSystemFactory::create(FileSystem::Extended, partition().firstSector(), partition().lastSector()));
	}

	KDialog::accept();
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

	dialogWidget().comboFileSystem().setEnabled(r != PartitionRole::Extended);
	partition().setRoles(PartitionRole(r));
	dialogWidget().partResizerWidget().update();
	updateHideAndShow();
}

void NewDialog::onFilesystemChanged(int idx)
{
	const FileSystem::Type t = FileSystem::typeForName(dialogWidget().comboFileSystem().itemText(idx));

	partition().deleteFileSystem();
	partition().setFileSystem(FileSystemFactory::create(t, partition().firstSector(), partition().lastSector()));

	setupConstraints();

	dialogWidget().partResizerWidget().updateLength(partition().length());

	updateHideAndShow();
}

void NewDialog::onLabelChanged(const QString& newLabel)
{
	partition().fileSystem().setLabel(newLabel);
}

void NewDialog::updateHideAndShow()
{
	// this is mostly copy'n'pasted from PartPropsDialog::updateHideAndShow()
	if (partition().roles().has(PartitionRole::Extended) || partition().fileSystem().supportSetLabel() == FileSystem::SupportNone)
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
		dialogWidget().label().setReadOnly(false);
		dialogWidget().noSetLabel().setVisible(false);
	}
}
