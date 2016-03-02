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

#include "config/advancedpagewidget.h"

#include <backend/corebackendmanager.h>

#include <util/helpers.h>

#include <QComboBox>

#include <config.h>

AdvancedPageWidget::AdvancedPageWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
    setupDialog();
}

QString AdvancedPageWidget::backend() const
{
    KService::List services = CoreBackendManager::self()->list();

    foreach(KService::Ptr p, services)
    if (p->name() == comboBackend().currentText())
        return p->library();

    return QString();
}

void AdvancedPageWidget::setBackend(const QString& name)
{
    KService::List services = CoreBackendManager::self()->list();

    foreach(KService::Ptr p, services)
    if (p->library() == name)
        comboBackend().setCurrentIndex(comboBackend().findText(p->name()));
}

void AdvancedPageWidget::setupDialog()
{
    KService::List services = CoreBackendManager::self()->list();
    foreach(KService::Ptr p, services)
    comboBackend().addItem(p->name());

    setBackend(Config::backend());
}
