/***************************************************************************
 *   Copyright (C) 2009 by Volker Lanz <vl@fidra.de>                       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#if !defined(PARTITIONMANAGERKCM__H)

#define PARTITIONMANAGERKCM__H

#include "ui_partitionmanagerkcmbase.h"

#include "util/globallog.h"

#include <kcmodule.h>
#include <kdebug.h>

class PartitionManagerWidget;
class KActionCollection;
class Device;
class KToolBar;

class PartitionManagerKCM : public KCModule, public Ui::PartitionManagerKCMBase
{
	Q_OBJECT
	Q_DISABLE_COPY(PartitionManagerKCM)

	public:
		PartitionManagerKCM(QWidget* parent, const QVariantList& args);
		virtual ~PartitionManagerKCM() {}

	public:
		void load() {}
		void save() {}

	protected:
		void setupConnections();
		void setupKCMWorkaround();

		PartitionManagerWidget& pmWidget() { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }
		ListOperations& listOperations() { Q_ASSERT(m_ListOperations); return *m_ListOperations; }
		QSplitter& splitterHorizontal() { Q_ASSERT(m_SplitterHorizontal); return *m_SplitterHorizontal; }
		QSplitter& splitterVertical() { Q_ASSERT(m_SplitterVertical); return *m_SplitterVertical; }
		KToolBar& toolBar() { Q_ASSERT(m_ToolBar); return *m_ToolBar; }

		KActionCollection* actionCollection() { return m_ActionCollection; }

	protected slots:
		void onNewLogMessage(Log::Level logLevel, const QString& s);
		void onStatusChanged();
		void onApplyClicked();

	private:
		KActionCollection* m_ActionCollection;
};


#endif
