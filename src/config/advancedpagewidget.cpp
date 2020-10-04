/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "config/advancedpagewidget.h"

#include <backend/corebackendmanager.h>
#include <util/helpers.h>

#include <QComboBox>
#include <QFileDialog>

#include <KPluginMetaData>

#include <config.h>
#include <kpmcore/core/softwareraid.h>

AdvancedPageWidget::AdvancedPageWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
    setupDialog();

    connect(selectRaidFileButton, &QPushButton::clicked, this, &AdvancedPageWidget::searchForRaidConfigFile);
}

QString AdvancedPageWidget::backend() const
{
    const auto backends = CoreBackendManager::self()->list();
    for (const auto &backend : backends)
        if (backend.name() == comboBackend().currentText())
            return backend.pluginId();

    return QString();
}

void AdvancedPageWidget::setBackend(const QString& name)
{
    const auto backends = CoreBackendManager::self()->list();
    for (const auto &backend : backends)
        if (backend.pluginId() == name)
            comboBackend().setCurrentIndex(comboBackend().findText(backend.name()));
}

void AdvancedPageWidget::setupDialog()
{
    const auto backends = CoreBackendManager::self()->list();
    for (const auto &backend : backends)
        comboBackend().addItem(backend.name());

    setBackend(Config::backend());
    setRaidConfigurationFile(Config::raidConfigurationFilePath());
}

QString AdvancedPageWidget::raidConfigurationFile() const
{
    return raidConfigFilePath->text().trimmed();
}

void AdvancedPageWidget::setRaidConfigurationFile(const QString &file)
{
    raidConfigFilePath->clear();
    raidConfigFilePath->insert(file);
}

void AdvancedPageWidget::searchForRaidConfigFile()
{
    QPointer<QFileDialog> dialog = new QFileDialog(this, QStringLiteral("Select Software RAID configuration file"),
                                                         QStringLiteral("/"));

    dialog->setFileMode(QFileDialog::FileMode::ExistingFile);
    dialog->setNameFilter(QStringLiteral("Configuration files (*.conf)"));

    auto updateConfig = [this](const QString& file){
        if (!file.isEmpty()) {
            raidConfigFilePath->clear();
            raidConfigFilePath->insert(file);
        }
     };

    connect(dialog, &QFileDialog::fileSelected, updateConfig);

    dialog->exec();
}
