/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/


#if !defined(FILESYSTEMCOLORSPAGEWIDGET_H)

#define FILESYSTEMCOLORSPAGEWIDGET_H

#include "ui_configurepagefilesystemcolors.h"

#include <QWidget>

class FileSystemColorsPageWidget : public QWidget, public Ui::ConfigurePageFileSystemColors
{
public:
    explicit FileSystemColorsPageWidget(QWidget* parent);
};

#endif
