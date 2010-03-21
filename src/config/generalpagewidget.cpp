/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#include "config/generalpagewidget.h"

#include "backend/corebackendmanager.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/helpers.h"

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
	comboDefaultFileSystem().setCurrentIndex(comboDefaultFileSystem().findText(FileSystem::nameForType(t)));
}

QString GeneralPageWidget::backend() const
{
	KService::List services = CoreBackendManager::self()->list();

	foreach(KService::Ptr p, services)
		if (p->name() == comboBackend().currentText())
			return p->library();

	return QString();
}

void GeneralPageWidget::setBackend(const QString& name)
{
	KService::List services = CoreBackendManager::self()->list();

	foreach(KService::Ptr p, services)
		if (p->library() == name)
			comboBackend().setCurrentIndex(comboBackend().findText(p->name()));
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

	setDefaultFileSystem(FileSystem::defaultFileSystem());

	KService::List services = CoreBackendManager::self()->list();
	foreach(KService::Ptr p, services)
		comboBackend().addItem(p->name());

	setBackend(Config::backend());
}
