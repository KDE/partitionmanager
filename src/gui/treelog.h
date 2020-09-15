/*
    SPDX-FileCopyrightText: 2009-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(TREELOG_H)

#define TREELOG_H

#include "ui_treelogbase.h"

#include <util/globallog.h>

#include <QWidget>

class QTreeWidget;

/** A tree for formatted log output.
    @author Volker Lanz <vl@fidra.de>
*/
class TreeLog: public QWidget, public Ui::TreeLogBase
{
    Q_DISABLE_COPY(TreeLog)

public:
    explicit TreeLog(QWidget* parent = nullptr);
    ~TreeLog();

public:
    void init();
    void onNewLogMessage(Log::Level logLevel, const QString& s);

    void onClearLog();
    void onSaveLog();

    QTreeWidget& treeLog() {
        Q_ASSERT(m_TreeLog);
        return *m_TreeLog;
    }

protected:
    void onHeaderContextMenu(const QPoint& pos);

    const QTreeWidget& treeLog() const {
        Q_ASSERT(m_TreeLog);
        return *m_TreeLog;
    }

    void loadConfig();
    void saveConfig() const;

private:
};

#endif
