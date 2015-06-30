/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#include "config/generalpagewidget.h"

#include <kpmcore/backend/corebackendmanager.h>
#include <kpmcore/fs/filesystemfactory.h>

#include <kpmcore/util/helpers.h>
#include "util/guihelpers.h"

#include <config.h>

GeneralPageWidget::GeneralPageWidget(QWidget* parent) :
	QWidget(parent)
{
	setupUi(this);
	setupDialog();
}

FileSystem::Type GeneralPageWidget::defaultFileSystem() const
{
	return FileSystem::typeForName(comboDefaultFileSystem().currentText());
}

void GeneralPageWidget::setDefaultFileSystem(FileSystem::Type t)
{
	const int idx = comboDefaultFileSystem().findText(FileSystem::nameForType(t));
	comboDefaultFileSystem().setCurrentIndex(idx != -1 ? idx : 0);
}

void GeneralPageWidget::setupDialog()
{
	QStringList fsNames;
	foreach (const FileSystem* fs, FileSystemFactory::map())
		if (fs->supportCreate() != FileSystem::cmdSupportNone && fs->type() != FileSystem::Extended)
			fsNames.append(fs->name());

	qSort(fsNames.begin(), fsNames.end(), caseInsensitiveLessThan);

	foreach (const QString& fsName, fsNames)
		comboDefaultFileSystem().addItem(createFileSystemColor(FileSystem::typeForName(fsName), 8), fsName);

	setDefaultFileSystem(GuiHelpers::defaultFileSystem());

	kcfg_shredSource->setId(radioButton, 0);
	kcfg_shredSource->setId(radioButton_2, 1);
	radioButton->setChecked(Config::shredSource() == Config::EnumShredSource::random);
	radioButton_2->setChecked(Config::shredSource() == Config::EnumShredSource::zeros);
}
