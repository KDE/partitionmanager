/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(APPLYPROGRESSDETAILSWIDGET_H)

#define APPLYPROGRESSDETAILSWIDGET_H

#include "ui_applyprogressdetailswidgetbase.h"

/** Details widget for the ProgressDialog.
    @author Volker Lanz <vl@fidra.de>
*/
class ApplyProgressDetailsWidget : public QWidget, public Ui::ApplyProgressDetailsWidgetBase
{
public:
    explicit ApplyProgressDetailsWidget(QWidget* parent) : QWidget(parent) {
        setupUi(this);
    }

public:
    QTextEdit& editReport() {
        Q_ASSERT(m_EditReport);
        return *m_EditReport;
    }
    QPushButton& buttonSave() {
        Q_ASSERT(m_ButtonSave);
        return *m_ButtonSave;
    }
    QPushButton& buttonBrowser() {
        Q_ASSERT(m_ButtonBrowser);
        return *m_ButtonBrowser;
    }
};

#endif
