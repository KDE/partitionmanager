/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(EDITMOUNTOPTIONSDIALOGWIDGET_H)

#define EDITMOUNTOPTIONSDIALOGWIDGET_H

#include "ui_editmountoptionsdialogwidgetbase.h"

#include <QWidget>

class QStringList;
class QPlainTextEdit;

class EditMountOptionsDialogWidget : public QWidget, public Ui::EditMountOptionsDialogWidgetBase
{
public:
    EditMountOptionsDialogWidget(QWidget* parent, const QStringList& options);

    QPlainTextEdit& editOptions() {
        return *m_EditOptions;
    }
};

#endif
