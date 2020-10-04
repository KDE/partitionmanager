/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/


#if !defined(GENERALPAGEWIDGET_H)

#define GENERALPAGEWIDGET_H

#include "ui_configurepagegeneral.h"

#include <fs/filesystem.h>

#include <QWidget>

class QString;

class GeneralPageWidget : public QWidget, public Ui::ConfigurePageGeneral
{
public:
    explicit GeneralPageWidget(QWidget* parent);

public:
    QComboBox& comboDefaultFileSystem() {
        return *m_ComboDefaultFileSystem;
    }
    const QComboBox& comboDefaultFileSystem() const {
        return *m_ComboDefaultFileSystem;
    }

    FileSystem::Type defaultFileSystem() const;
    void setDefaultFileSystem(FileSystem::Type t);

private:
    void setupDialog();
};

#endif

