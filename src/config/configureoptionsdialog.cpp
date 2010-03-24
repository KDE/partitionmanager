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
#include "config/generalpagewidget.h"
#include "config/filesystemcolorspagewidget.h"

#include "backend/corebackendmanager.h"

#include "core/operationstack.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/helpers.h"

#include "ui_configurepagefilesystemcolors.h"

#include <kiconloader.h>
#include <kservice.h>
#include <kmessagebox.h>

#include <config.h>

ConfigureOptionsDialog::ConfigureOptionsDialog(QWidget* parent, const OperationStack& ostack, const QString& name) :
	KConfigDialog(parent, name, Config::self()),
	m_GeneralPageWidget(new GeneralPageWidget(this)),
	m_FileSystemColorsPageWidget(new FileSystemColorsPageWidget(this)),
	m_OperationStack(ostack)
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

void ConfigureOptionsDialog::onComboBackendActivated(int)
{
	if (operationStack().size() == 0 || KMessageBox::warningContinueCancel(this,
			i18nc("@info",
				"<para>Do you really want to change the backend?</para>"
				"<para><warning>This will also rescan devices and thus clear the list of pending operations.</warning></para>"),
			i18nc("@title:window", "Really Change Backend?"),
			KGuiItem(i18nc("@action:button", "Change the Backend")),
			KGuiItem(i18nc("@action:button", "Do Not Change the Backend")), "reallyChangeBackend") == KMessageBox::Continue)
	{
		settingsChangedSlot();
	}
	else
		generalPageWidget().setBackend(CoreBackendManager::defaultBackendName());
}
