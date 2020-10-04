/*
    SPDX-FileCopyrightText: 2009-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/listoperations.h"

#include <ops/operation.h>

#include <util/globallog.h>
#include <util/capacity.h>

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
        QListWidgetItem* item = new QListWidgetItem(QIcon::fromTheme(op->iconName()), op->description());
        item->setToolTip(op->description());
        listOperations().addItem(item);
    }

    listOperations().scrollToBottom();
}
