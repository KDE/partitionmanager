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

#include "gui/configureoptionsdialog.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/helpers.h"

#include "ui_configurepagegeneral.h"
#include "ui_configurepagefilesystemcolors.h"

#include <kiconloader.h>
#include <config.h>

class GeneralPageWidget : public QWidget, public Ui::ConfigurePageGeneral
{
	public:
		GeneralPageWidget(QWidget* parent) : QWidget(parent) { setupUi(this); setupDialog(); }

	public:
		QComboBox& comboDefaultFileSystem() { return *m_ComboDefaultFileSystem; }

	private:
		void setupDialog()
		{
			QStringList fsNames;
			foreach (const FileSystem* fs, FileSystemFactory::map())
				if (fs->supportCreate() != FileSystem::cmdSupportNone && fs->type() != FileSystem::Extended)
					fsNames.append(fs->name());

			qSort(fsNames.begin(), fsNames.end(), caseInsensitiveLessThan);

			foreach (const QString& fsName, fsNames)
				comboDefaultFileSystem().addItem(createFileSystemColor(FileSystem::typeForName(fsName), 8), fsName);

			const QString selected = FileSystem::nameForType(FileSystem::defaultFileSystem());
			comboDefaultFileSystem().setCurrentIndex(comboDefaultFileSystem().findText(selected));
		}

};

class FileSystemColorsPageWidget : public QWidget, public Ui::ConfigurePageFileSystemColors
{
	public:
		FileSystemColorsPageWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }
};

ConfigureOptionsDialog::ConfigureOptionsDialog(QWidget* parent, const QString& name, KConfigSkeleton* cfg) :
	KConfigDialog(parent, name, cfg),
	m_GeneralPageWidget(new GeneralPageWidget(this)),
	m_FileSystemColorsPageWidget(new FileSystemColorsPageWidget(this))
{
	setFaceType(List);

	KPageWidgetItem* item = NULL;

	item = addPage(&generalPageWidget(), i18n("General"), QString(), i18n("General Settings"));
	item->setIcon(KIcon(DesktopIcon("configure")));

	item = addPage(&fileSystemColorsPageWidget(), i18n("File System Colors"), QString(), i18n("File System Color Settings"));
	item->setIcon(KIcon(DesktopIcon("format-fill-color")));

	restoreDialogSize(KConfigGroup(KGlobal::config(), "configureOptionsDialog"));
}

void ConfigureOptionsDialog::updateSettings()
{
	Config::setDefaultFileSystem(FileSystem::typeForName(generalPageWidget().comboDefaultFileSystem().currentText()));
}

/** Destroys a ConfigureOptionsDialog instance */
ConfigureOptionsDialog::~ConfigureOptionsDialog()
{
	KConfigGroup kcg(KGlobal::config(), "configureOptionsDialog");
	saveDialogSize(kcg);
}
