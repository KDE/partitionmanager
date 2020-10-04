/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(APPLYPROGRESSDIALOGWIDGET_H)

#define APPLYPROGRESSDIALOGWIDGET_H

#include "ui_applyprogressdialogwidgetbase.h"

/** Central widget for the ProgressDialog.
    @author Volker Lanz <vl@fidra.de>
*/
class ApplyProgressDialogWidget : public QWidget, public Ui::ApplyProgressDialogWidgetBase
{
public:
    explicit ApplyProgressDialogWidget(QWidget* parent);

public:
    QTreeWidget& treeTasks() {
        Q_ASSERT(m_TreeTasks);
        return *m_TreeTasks;
    }
    QProgressBar& progressTotal() {
        Q_ASSERT(m_ProgressTotal);
        return *m_ProgressTotal;
    }
    QProgressBar& progressSub() {
        Q_ASSERT(m_ProgressSub);
        return *m_ProgressSub;
    }
    QLabel& status() {
        Q_ASSERT(m_LabelStatus);
        return *m_LabelStatus;
    }
    QLabel& totalTime() {
        Q_ASSERT(m_LabelTime);
        return *m_LabelTime;
    }
};

#endif
