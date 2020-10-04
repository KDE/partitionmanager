/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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

#include <KLocalizedString>
#include <KMessageBox>

#include <QtGlobal>
#include <QIcon>

#include <config.h>
#include <kpmcore/core/softwareraid.h>

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
    connect(&advancedPageWidget().raidConfigurationLine(), &QLineEdit::textChanged, this, &ConfigureOptionsDialog::onRaidConfigFilePathActivated);

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

    if (advancedPageWidget().raidConfigurationFile() != Config::raidConfigurationFilePath()) {
        Config::setRaidConfigurationFilePath(advancedPageWidget().raidConfigurationFile());
        changed = true;
    }

    if (changed)
        Q_EMIT KConfigDialog::settingsChanged(i18n("General Settings"));
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

        kcItem = Config::self()->findItem(QStringLiteral("raidConfigurationFilePath"));
        result = result || !kcItem->isEqual(advancedPageWidget().raidConfigurationFile());
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

    if (advancedPageWidget().isVisible()) {
        advancedPageWidget().setBackend(CoreBackendManager::defaultBackendName());
        advancedPageWidget().setRaidConfigurationFile(SoftwareRAID::getDefaultRaidConfigFile());
    }

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
