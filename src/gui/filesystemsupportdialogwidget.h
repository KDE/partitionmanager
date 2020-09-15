/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(FILESYSTEMSUPPORTDIALOGWIDGET_H)

#define FILESYSTEMSUPPORTDIALOGWIDGET_H

#include "ui_filesystemsupportdialogwidgetbase.h"

class FileSystemSupportDialogWidget : public QWidget, public Ui::FileSystemSupportDialogWidgetBase
{
public:
    explicit FileSystemSupportDialogWidget(QWidget* parent);

public:
    QTreeWidget& tree() {
        Q_ASSERT(m_Tree);
        return *m_Tree;
    }
    const QTreeWidget& tree() const {
        Q_ASSERT(m_Tree);
        return *m_Tree;
    }
    QPushButton& buttonRescan() {
        Q_ASSERT(m_ButtonRescan);
        return *m_ButtonRescan;
    }
    const QPushButton& buttonRescan() const {
        Q_ASSERT(m_ButtonRescan);
        return *m_ButtonRescan;
    }
};

#endif
