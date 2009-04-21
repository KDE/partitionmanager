/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#if !defined(LISTOPERATIONS__H)

#define LISTOPERATIONS__H

#include "util/libpartitionmanagerexport.h"

#include "ui_listoperationsbase.h"

#include <QWidget>

#include <kdebug.h>

class Operation;
class QPoint;
class PartitionManagerWidget;
class KActionCollection;

/** @brief A list of pending operations.

	@author vl@fidra.de
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT ListOperations : public QWidget, public Ui::ListOperationsBase
{
	Q_OBJECT

	public:
		ListOperations(QWidget* parent);

	public:
		void init(KActionCollection* coll, PartitionManagerWidget* pm_widget) { m_ActionCollection = coll; m_PartitionManagerWidget = pm_widget; }

	protected:
		KActionCollection* actionCollection() { return m_ActionCollection; }

		PartitionManagerWidget& pmWidget() { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }
		const PartitionManagerWidget& pmWidget() const { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }

		QListWidget& listOperations() { Q_ASSERT(m_ListOperations); return *m_ListOperations; }
		const QListWidget& listOperations() const { Q_ASSERT(m_ListOperations); return *m_ListOperations; }

	protected slots:
		void on_m_ListOperations_customContextMenuRequested(const QPoint& pos);
		void updateOperations();

	private:
		KActionCollection* m_ActionCollection;
		PartitionManagerWidget* m_PartitionManagerWidget;
};

#endif

