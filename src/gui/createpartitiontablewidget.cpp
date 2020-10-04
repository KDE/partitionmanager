/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/createpartitiontablewidget.h"

#include <QIcon>

CreatePartitionTableWidget::CreatePartitionTableWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
    QIcon icon = QIcon::fromTheme(QStringLiteral("dialog-warning"));
    iconLabel().setPixmap(icon.pixmap(32, 32));
}
