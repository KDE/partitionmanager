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

#if !defined(TREELOG__H)

#define TREELOG__H

#include "util/libpartitionmanagerexport.h"

#include "ui_treelogbase.h"

#include "util/globallog.h"

#include <QWidget>

#include <kdebug.h>

class PartitionManagerWidget;
class KActionCollection;
class QTreeWidget;

/** @brief A tree for formatted log output.
	@author vl@fidra.de
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT TreeLog: public QWidget, public Ui::TreeLogBase
{
	Q_OBJECT
	Q_DISABLE_COPY(TreeLog)

	public:
		TreeLog(QWidget* parent);

	public:
		void init(KActionCollection* coll, PartitionManagerWidget* pm_widget) { m_ActionCollection = coll; m_PartitionManagerWidget = pm_widget; }

	public slots:
		void onNewLogMessage(log::Level logLevel, const QString& s);

	protected:
		QTreeWidget& treeLog() { Q_ASSERT(m_TreeLog); return *m_TreeLog; }
		const QTreeWidget& treeLog() const { Q_ASSERT(m_TreeLog); return *m_TreeLog; }

		PartitionManagerWidget& pmWidget() { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }
		const PartitionManagerWidget& pmWidget() const { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }
		KActionCollection* actionCollection() { return m_ActionCollection; }

	protected slots:

	private:
		KActionCollection* m_ActionCollection;
		PartitionManagerWidget* m_PartitionManagerWidget;
};

#endif

