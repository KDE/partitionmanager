/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
 *  Copyright (C) 2018 by Caio Carvalho <caiojcarvalho@gmail.com>        *
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
