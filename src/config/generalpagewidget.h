/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(GENERALPAGEWIDGET__H)

#define GENERALPAGEWIDGET__H

#include "ui_configurepagegeneral.h"

#include <kpmcore/fs/filesystem.h>

#include <QWidget>

class QString;

class GeneralPageWidget : public QWidget, public Ui::ConfigurePageGeneral
{
public:
    GeneralPageWidget(QWidget* parent);

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

