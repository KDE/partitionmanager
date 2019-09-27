/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "config/configureoptionsdialog.h"
#include "config/generalpagewidget.h"
#include "config/filesystemcolorspagewidget.h"
#include "config/advancedpagewidget.h"

#include <backend/corebackendmanager.h>

#include <core/operationstack.h>

#include <fs/filesystem.h>
#include <fs/filesystemfactory.h>

#include <util/helpers.h>
#include "util/guihelpers.h"

#include "ui_configurepagefilesystemcolors.h"

#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>

#include <QtGlobal>
#include <QIcon>

#include <config.h>

ConfigureOptionsDialog::ConfigureOptionsDialog(QWidget* parent, const OperationStack& ostack, const QString& name) :
    KConfigDialog(parent, name, Config::self()),
    m_GeneralPageWidget(new GeneralPageWidget(this)),
    m_FileSystemColorsPageWidget(new FileSystemColorsPageWidget(this)),
    m_AdvancedPageWidget(new AdvancedPageWidget(this)),
    m_OperationStack(ostack)
{
    setFaceType(List);

    addPage(&generalPageWidget(), xi18nc("@title:tab general application settings", "General"), QStringLiteral("partitionmanager"), i18n("General Settings"));

    connect(&generalPageWidget().comboDefaultFileSystem(), qOverload<int>(&QComboBox::activated), this, &ConfigureOptionsDialog::onComboDefaultFileSystemActivated);
    connect(generalPageWidget().radioButton, &QRadioButton::toggled, this, &ConfigureOptionsDialog::onShredSourceActivated);

    addPage(&fileSystemColorsPageWidget(), xi18nc("@title:tab", "File System Colors"), QStringLiteral("preferences-desktop-color"), i18n("File System Color Settings"));

    addPage(&advancedPageWidget(), xi18nc("@title:tab advanced application settings", "Advanced"), QStringLiteral("preferences-other"), i18n("Advanced Settings"));

    connect(&advancedPageWidget().comboBackend(), qOverload<int>(&QComboBox::activated), this, &ConfigureOptionsDialog::onComboDefaultFileSystemActivated);

    KConfigGroup kcg(KSharedConfig::openConfig(), "configureOptionsDialogs");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a ConfigureOptionsDialog instance */
ConfigureOptionsDialog::~ConfigureOptionsDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), "configureOptionsDialog");
    kcg.writeEntry("Geometry", saveGeometry());
}

void ConfigureOptionsDialog::updateSettings()
{
    KConfigDialog::updateSettings();

    bool changed = false;

    if (generalPageWidget().defaultFileSystem() != static_cast<FileSystem::Type>(Config::defaultFileSystem())) {
        Config::setDefaultFileSystem(static_cast<int>(generalPageWidget().defaultFileSystem()));
        changed = true;
    }

    if (generalPageWidget().radioButton->isChecked() != (Config::shredSource() == Config::EnumShredSource::random)) {
        qDebug() << "updateSettings: " << generalPageWidget().kcfg_shredSource->checkedId();
        Config::setShredSource(generalPageWidget().kcfg_shredSource->checkedId());
        changed = true;
    }

    if (advancedPageWidget().backend() != Config::backend()) {
        Config::setBackend(advancedPageWidget().backend());
        changed = true;
    }

    if (changed)
        emit KConfigDialog::settingsChanged(i18n("General Settings"));
}

bool ConfigureOptionsDialog::hasChanged()
{
    bool result = KConfigDialog::hasChanged();

    KConfigSkeletonItem* kcItem = Config::self()->findItem(QStringLiteral("defaultFileSystem"));
    result = result || !kcItem->isEqual(static_cast<int>(generalPageWidget().defaultFileSystem()));
    result = result || (generalPageWidget().kcfg_shredSource->checkedId() != Config::shredSource());

    if (advancedPageWidget().isVisible()) {
        kcItem = Config::self()->findItem(QStringLiteral("backend"));
        result = result || !kcItem->isEqual(advancedPageWidget().backend());
    }

    return result;
}

bool ConfigureOptionsDialog::isDefault()
{
    bool result = KConfigDialog::isDefault();

    if (result) {
        const bool useDefaults = Config::self()->useDefaults(true);
        result = !hasChanged();
        Config::self()->useDefaults(useDefaults);
    }

    return result;
}

void ConfigureOptionsDialog::updateWidgetsDefault()
{
    bool useDefaults = Config::self()->useDefaults(true);
    generalPageWidget().setDefaultFileSystem(GuiHelpers::defaultFileSystem());
    generalPageWidget().radioButton->setChecked(true);

    if (advancedPageWidget().isVisible())
        advancedPageWidget().setBackend(CoreBackendManager::defaultBackendName());

    Config::self()->useDefaults(useDefaults);
}

void ConfigureOptionsDialog::onComboBackendActivated(int)
{
    Q_ASSERT(advancedPageWidget().isVisible());

    if (operationStack().size() == 0 || KMessageBox::warningContinueCancel(this,
            xi18nc("@info",
                   "<para>Do you really want to change the backend?</para>"
                   "<para><warning>This will also rescan devices and thus clear the list of pending operations.</warning></para>"),
            xi18nc("@title:window", "Really Change Backend?"),
            KGuiItem(xi18nc("@action:button", "Change the Backend"), QStringLiteral("arrow-right")),
            KGuiItem(xi18nc("@action:button", "Do Not Change the Backend"), QStringLiteral("dialog-cancel")), QStringLiteral("reallyChangeBackend")) == KMessageBox::Continue) {
        settingsChangedSlot();
    } else
        advancedPageWidget().setBackend(CoreBackendManager::defaultBackendName());
}
