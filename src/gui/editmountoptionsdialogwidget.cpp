/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/editmountoptionsdialogwidget.h"

#include <QStringList>

EditMountOptionsDialogWidget::EditMountOptionsDialogWidget(QWidget* parent, const QStringList& options) :
    QWidget(parent)
{
    setupUi(this);

    for (const auto &o : options)
        editOptions().appendPlainText(o);
}
