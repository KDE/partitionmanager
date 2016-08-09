/*************************************************************************
 *  Copyright (C) 2008, 2009 by Volker Lanz <vl@fidra.de>                *
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

#include "gui/listoperations.h"

#include <ops/operation.h>

#include <util/globallog.h>
#include <util/capacity.h>

#include <KIconLoader>

/** Creates a new ListOperations instance.
    @param parent the parent widget
*/
ListOperations::ListOperations(QWidget* parent) :
    QWidget(parent),
    Ui::ListOperationsBase(),
    m_ActionCollection(nullptr)
{
    setupUi(this);
}

void ListOperations::updateOperations(const OperationStack::Operations& ops)
{
    listOperations().clear();

    for (const auto &op : ops) {
        QListWidgetItem* item = new QListWidgetItem(QIcon::fromTheme(op->iconName()).pixmap(IconSize(KIconLoader::Small)), op->description());
        item->setToolTip(op->description());
        listOperations().addItem(item);
    }

    listOperations().scrollToBottom();
}
