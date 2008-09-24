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

	if (partitionRoles() & PartitionRole::Primary)
		dialogWidget().comboRole().addItem(i18nc("@item:inlistbox partition role", "Primary"));
	if (partitionRoles() & PartitionRole::Extended)
		dialogWidget().comboRole().addItem(i18nc("@item:inlistbox partition role", "Extended"));
	if (partitionRoles() & PartitionRole::Logical)
		dialogWidget().comboRole().addItem(i18nc("@item:inlistbox partition role", "Logical"));

	SizeDialogBase::setupDialog();

	// don't move these above the call to parent's setupDialog, because only after that has
	// run there is a valid partition set in the part resizer widget and they will need that.
	onRoleChanged(0);
	onFilesystemChanged(0);
}

void NewDialog::setupConnections()
{
	connect(&dialogWidget().comboRole(), SIGNAL(currentIndexChanged(int)), SLOT(onRoleChanged(int)));
	connect(&dialogWidget().comboFileSystem(), SIGNAL(currentIndexChanged(int)), SLOT(onFilesystemChanged(int)));

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

void NewDialog::onRoleChanged(int idx)
{
	PartitionRole::Roles r = PartitionRole::None;

	const QString itemText = dialogWidget().comboRole().itemText(idx);
	if (itemText == i18nc("@item:inlistbox partition role", "Primary"))
		r = PartitionRole::Primary;
	if (itemText == i18nc("@item:inlistbox partition role", "Extended"))
		r = PartitionRole::Extended;
	if (itemText == i18nc("@item:inlistbox partition role", "Logical"))
		r = PartitionRole::Logical;

	dialogWidget().comboFileSystem().setEnabled(r != PartitionRole::Extended);
	partition().setRoles(PartitionRole(r));
	dialogWidget().partResizerWidget().update();
}

void NewDialog::onFilesystemChanged(int idx)
{
	const FileSystem::Type t = FileSystem::typeForName(dialogWidget().comboFileSystem().itemText(idx));

	partition().deleteFileSystem();
	partition().setFileSystem(FileSystemFactory::create(t, partition().firstSector(), partition().lastSector()));

	setupConstraints();

	dialogWidget().partResizerWidget().updateLength(partition().length());
}
