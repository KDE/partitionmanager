/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(ADVANCEDPAGEWIDGET_H)

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

    QString backend() const;
    void setBackend(const QString& name);

private:
    void setupDialog();
};

#endif

