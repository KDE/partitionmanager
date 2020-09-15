/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/applyprogressdialogwidget.h"

ApplyProgressDialogWidget::ApplyProgressDialogWidget(QWidget* parent) : QWidget(parent) {
    setupUi(this);
    m_TreeTasks->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
