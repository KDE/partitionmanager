/*************************************************************************
 *  Copyright (C) 2008, 2009 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(TREELOG__H)

#define TREELOG__H

#include "ui_treelogbase.h"

#include <util/globallog.h>

#include <QWidget>

class QTreeWidget;

/** A tree for formatted log output.
    @author Volker Lanz <vl@fidra.de>
*/
class TreeLog: public QWidget, public Ui::TreeLogBase
{
    Q_OBJECT
    Q_DISABLE_COPY(TreeLog)

public:
    TreeLog(QWidget* parent = nullptr);
    ~TreeLog();

Q_SIGNALS:
    void contextMenuRequested(const QPoint&);

public:
    void init();
    void onNewLogMessage(Log::Level logLevel, const QString& s);

    void onClearLog();
    void onSaveLog();

protected:
    void on_m_TreeLog_customContextMenuRequested(const QPoint& pos);
    void onHeaderContextMenu(const QPoint& pos);

    QTreeWidget& treeLog() {
        Q_ASSERT(m_TreeLog);
        return *m_TreeLog;
    }
    const QTreeWidget& treeLog() const {
        Q_ASSERT(m_TreeLog);
        return *m_TreeLog;
    }

    void loadConfig();
    void saveConfig() const;

private:
};

#endif
