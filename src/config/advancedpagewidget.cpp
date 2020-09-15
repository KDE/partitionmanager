/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "config/advancedpagewidget.h"

#include <backend/corebackendmanager.h>
#include <util/helpers.h>

#include <QComboBox>

#include <KPluginMetaData>

#include <config.h>

AdvancedPageWidget::AdvancedPageWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
    setupDialog();
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
}
