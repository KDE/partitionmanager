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

#include "ui_configurepagegeneral.h"
#include "ui_configurepagefilesystemcolors.h"
#include "ui_configurepageadvanced.h"

class GeneralPageWidget : public QWidget, public Ui::ConfigurePageGeneral
{
	public:
		GeneralPageWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }
};

class FileSystemColorsPageWidget : public QWidget, public Ui::ConfigurePageFileSystemColors
{
	public:
		FileSystemColorsPageWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }
};

class AdvancedPageWidget : public QWidget, public Ui::ConfigurePageAdvanced
{
	public:
		AdvancedPageWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }
};

ConfigureOptionsDialog::ConfigureOptionsDialog(QWidget* parent, const QString& name, KConfigSkeleton* cfg) :
	KConfigDialog(parent, name, cfg)
{
	setFaceType(KPageDialog::Tabbed);
	addPage(new GeneralPageWidget(this), i18n("General"), QString(), i18n("General Settings"));
	addPage(new FileSystemColorsPageWidget(this), i18n("File System Colors"), QString(), i18n("File System Color Settings"));
	addPage(new AdvancedPageWidget(this), i18n("Advanced"), QString(), i18n("Advanced Settings"));

	restoreDialogSize(KConfigGroup(KGlobal::config(), "configureOptionsDialog"));
}

/** Destroys a ConfigureOptionsDialog instance */
ConfigureOptionsDialog::~ConfigureOptionsDialog()
{
	KConfigGroup kcg(KGlobal::config(), "configureOptionsDialog");
	saveDialogSize(kcg);
}
