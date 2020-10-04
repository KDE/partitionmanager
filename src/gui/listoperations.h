/*
    SPDX-FileCopyrightText: 2009-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/


#if !defined(LISTOPERATIONS_H)

#define LISTOPERATIONS_H

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
    Q_DISABLE_COPY(ListOperations)

public:
    explicit ListOperations(QWidget* parent = nullptr);

public:
    void setActionCollection(KActionCollection* coll) {
        m_ActionCollection = coll;
    }

    void updateOperations(const OperationStack::Operations& ops);

    QListWidget& listOperations() {
        Q_ASSERT(m_ListOperations);
        return *m_ListOperations;
    }

protected:
    KActionCollection* actionCollection() {
        return m_ActionCollection;
    }

    const QListWidget& listOperations() const {
        Q_ASSERT(m_ListOperations);
        return *m_ListOperations;
    }

    void customContextMenuRequested(const QPoint& pos);

private:
    KActionCollection* m_ActionCollection;
};

#endif

