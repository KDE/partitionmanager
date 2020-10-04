/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef ADVANCEDPAGEWIDGET_H
#define ADVANCEDPAGEWIDGET_H

#include "ui_configurepageadvanced.h"

#include <fs/filesystem.h>

#include <QWidget>

class QString;
class QComboBox;

class AdvancedPageWidget : public QWidget, public Ui::ConfigurePageAdvanced
{
public:
    explicit AdvancedPageWidget(QWidget* parent);

public:
    QComboBox& comboBackend() {
        return *m_ComboBackend;
    }
    const QComboBox& comboBackend() const {
        return *m_ComboBackend;
    }
    const QLineEdit& raidConfigurationLine() const {
        return *raidConfigFilePath;
    }

    QString backend() const;
    void setBackend(const QString& name);

    QString raidConfigurationFile() const;
    void setRaidConfigurationFile(const QString& file);

protected Q_SLOTS:
    void searchForRaidConfigFile();

private:
    void setupDialog();
};

#endif

