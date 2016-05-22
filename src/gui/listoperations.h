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

#if !defined(LISTOPERATIONS__H)

#define LISTOPERATIONS__H

#include <core/operationstack.h>

#include "ui_listoperationsbase.h"

#include <QWidget>

class Operation;
class QPoint;
class KActionCollection;

/** A list of pending operations.

    @author Volker Lanz <vl@fidra.de>
*/
class ListOperations : public QWidget, public Ui::ListOperationsBase
{
    Q_OBJECT
    Q_DISABLE_COPY(ListOperations)

public:
    ListOperations(QWidget* parent = nullptr);

Q_SIGNALS:
    void contextMenuRequested(const QPoint&);

public:
    void setActionCollection(KActionCollection* coll) {
        m_ActionCollection = coll;
    }

    void updateOperations(const OperationStack::Operations& ops);

protected:
    KActionCollection* actionCollection() {
        return m_ActionCollection;
    }

    QListWidget& listOperations() {
        Q_ASSERT(m_ListOperations);
        return *m_ListOperations;
    }
    const QListWidget& listOperations() const {
        Q_ASSERT(m_ListOperations);
        return *m_ListOperations;
    }

protected Q_SLOTS:
    void on_m_ListOperations_customContextMenuRequested(const QPoint& pos);

private:
    KActionCollection* m_ActionCollection;
};

#endif

