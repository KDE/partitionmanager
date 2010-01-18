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

#include "gui/filesystemsupportdialog.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"


#include <kdebug.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kiconloader.h>

/** Creates a new FileSystemSupportDialog
	@param parent the parent object
*/
FileSystemSupportDialog::FileSystemSupportDialog(QWidget* parent) :
	KDialog(parent),
	m_FileSystemSupportDialogWidget(new FileSystemSupportDialogWidget(this))
{
	setMainWidget(&dialogWidget());
	setCaption(i18nc("@title:window", "File System Support"));
	setButtons(KDialog::Ok);

 	setupDialog();
	setupConnections();

	restoreDialogSize(KConfigGroup(KGlobal::config(), "fileSystemSupportDialog"));
}

/** Destroys a FileSystemSupportDialog */
FileSystemSupportDialog::~FileSystemSupportDialog()
{
	KConfigGroup kcg(KGlobal::config(), "fileSystemSupportDialog");
	saveDialogSize(kcg);
}

QSize FileSystemSupportDialog::sizeHint() const
{
	return QSize(690, 490);
}

void FileSystemSupportDialog::setupDialog()
{
	QPixmap yes(BarIcon("dialog-ok"));
	QPixmap no(BarIcon("dialog-error"));

	dialogWidget().tree().clear();

	foreach(const FileSystem* fs, FileSystemFactory::map())
	{
		if (fs->type() == FileSystem::Unknown || fs->type() == FileSystem::Extended)
			continue;

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

		dialogWidget().tree().addTopLevelItem(item);
	}

	for(int i = 0; i < dialogWidget().tree().columnCount(); i++)
		dialogWidget().tree().resizeColumnToContents(i);

	dialogWidget().tree().sortItems(0, Qt::AscendingOrder);
}

void FileSystemSupportDialog::setupConnections()
{
	connect(&dialogWidget().buttonRescan(), SIGNAL(clicked()), SLOT(onButtonRescanClicked()));
}

void FileSystemSupportDialog::onButtonRescanClicked()
{
	FileSystemFactory::init();
	setupDialog();
}
