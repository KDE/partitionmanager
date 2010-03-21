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

#include "config/configureoptionsdialog.h"

#include "backend/corebackendmanager.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/helpers.h"

#include "ui_configurepagegeneral.h"
#include "ui_configurepagefilesystemcolors.h"

#include <kiconloader.h>
#include <kservice.h>

#include <config.h>

class GeneralPageWidget : public QWidget, public Ui::ConfigurePageGeneral
{
	public:
		GeneralPageWidget(QWidget* parent) : QWidget(parent) { setupUi(this); setupDialog(); }

	public:
		KComboBox& comboDefaultFileSystem() { return *m_ComboDefaultFileSystem; }
		const KComboBox& comboDefaultFileSystem() const { return *m_ComboDefaultFileSystem; }

		KComboBox& comboBackend() { return *m_ComboBackend; }
		const KComboBox& comboBackend() const { return *m_ComboBackend; }

		FileSystem::Type defaultFileSystem() const
		{
			return FileSystem::typeForName(comboDefaultFileSystem().currentText());
		}

		void setDefaultFileSystem(FileSystem::Type t)
		{
			comboDefaultFileSystem().setCurrentIndex(comboDefaultFileSystem().findText(FileSystem::nameForType(t)));
		}

		QString backend() const
		{
			KService::List services = CoreBackendManager::self()->list();

			foreach(KService::Ptr p, services)
				if (p->name() == comboBackend().currentText())
					return p->library();

			return QString();
		}

		void setBackend(const QString& name)
		{
			KService::List services = CoreBackendManager::self()->list();

			foreach(KService::Ptr p, services)
				if (p->library() == name)
					comboBackend().setCurrentIndex(comboBackend().findText(p->name()));
		}

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

			setDefaultFileSystem(FileSystem::defaultFileSystem());

			KService::List services = CoreBackendManager::self()->list();
			foreach(KService::Ptr p, services)
				comboBackend().addItem(p->name());

			setBackend(Config::backend());
		}
};

class FileSystemColorsPageWidget : public QWidget, public Ui::ConfigurePageFileSystemColors
{
	public:
		FileSystemColorsPageWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }
};

ConfigureOptionsDialog::ConfigureOptionsDialog(QWidget* parent, const QString& name) :
	KConfigDialog(parent, name, Config::self()),
	m_GeneralPageWidget(new GeneralPageWidget(this)),
	m_FileSystemColorsPageWidget(new FileSystemColorsPageWidget(this))
{
	setFaceType(List);

	KPageWidgetItem* item = NULL;

	item = addPage(&generalPageWidget(), i18nc("@title:tab general application settings", "General"), QString(), i18n("General Settings"));
	item->setIcon(KIcon(DesktopIcon("configure")));

	connect(&generalPageWidget().comboDefaultFileSystem(), SIGNAL(activated(int)), SLOT(onComboDefaultFileSystemActivated(int)));
	connect(&generalPageWidget().comboBackend(), SIGNAL(activated(int)), SLOT(onComboBackendActivated(int)));

	item = addPage(&fileSystemColorsPageWidget(), i18nc("@title:tab", "File System Colors"), QString(), i18n("File System Color Settings"));
	item->setIcon(KIcon(DesktopIcon("format-fill-color")));

	restoreDialogSize(KConfigGroup(KGlobal::config(), "configureOptionsDialog"));
}

/** Destroys a ConfigureOptionsDialog instance */
ConfigureOptionsDialog::~ConfigureOptionsDialog()
{
	KConfigGroup kcg(KGlobal::config(), "configureOptionsDialog");
	saveDialogSize(kcg);
}

void ConfigureOptionsDialog::updateSettings()
{
	KConfigDialog::updateSettings();

	bool changed = false;

	if (generalPageWidget().defaultFileSystem() != Config::defaultFileSystem())
	{
		Config::setDefaultFileSystem(generalPageWidget().defaultFileSystem());
		changed = true;
	}

	if (generalPageWidget().backend() != Config::backend())
	{
		Config::setBackend(generalPageWidget().backend());
		changed = true;
	}

	if (changed)
		emit KConfigDialog::settingsChanged(i18n("General Settings"));
}

bool ConfigureOptionsDialog::hasChanged()
{
	bool result = KConfigDialog::hasChanged();

	KConfigSkeletonItem* kcItem = Config::self()->findItem("defaultFileSystem");
	result = result || !kcItem->isEqual(generalPageWidget().defaultFileSystem());

	kcItem = Config::self()->findItem("backend");
	result = result || !kcItem->isEqual(generalPageWidget().backend());

	return result;
}

bool ConfigureOptionsDialog::isDefault()
{
	bool result = KConfigDialog::isDefault();

	if (result)
	{
		const bool useDefaults = Config::self()->useDefaults(true);
		result = !hasChanged();
		Config::self()->useDefaults(useDefaults);
	}

	return result;
}

void ConfigureOptionsDialog::updateWidgetsDefault()
{
	bool useDefaults = Config::self()->useDefaults(true);
	generalPageWidget().setDefaultFileSystem(FileSystem::defaultFileSystem());
	generalPageWidget().setBackend(CoreBackendManager::defaultBackendName());
	Config::self()->useDefaults(useDefaults);
}
