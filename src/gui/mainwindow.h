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

#if !defined(MAINWINDOW__H)

#define MAINWINDOW__H

#include "util/libpartitionmanagerexport.h"

#include "ui_mainwindowbase.h"

#include <kxmlguiwindow.h>

class QWidget;
class QLabel;
class InfoPane;
class QCloseEvent;
class QEvent;
class Device;
class KActionCollection;

/** @brief The application's main window.

	@author vl@fidra.de
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT MainWindow : public KXmlGuiWindow, public Ui::MainWindowBase
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget* parent = NULL, KActionCollection* coll = NULL);

	protected:
		void setupActions();
		void setupConnections();
		void setupStatusBar();
		void loadConfig();
		void saveConfig() const;
		void updateWindowTitle();

		KActionCollection* actionCollection() const { return m_ActionCollection != NULL ? m_ActionCollection : KXmlGuiWindow::actionCollection(); }

		InfoPane& infoPane() { Q_ASSERT(m_InfoPane); return *m_InfoPane; }

		PartitionManagerWidget& pmWidget() { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }
		const PartitionManagerWidget& pmWidget() const { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }

		ListDevices& listDevices() { Q_ASSERT(m_ListDevices); return *m_ListDevices; }
		const ListDevices& listDevices() const { Q_ASSERT(m_ListDevices); return *m_ListDevices; }

		ListOperations& listOperations() { Q_ASSERT(m_ListOperations); return *m_ListOperations; }
		const ListOperations& listOperations() const { Q_ASSERT(m_ListOperations); return *m_ListOperations; }

		TreeLog& treeLog() { Q_ASSERT(m_TreeLog); return *m_TreeLog; }
		const TreeLog& treeLog() const { Q_ASSERT(m_TreeLog); return *m_TreeLog; }

		QDockWidget& dockInformation() { Q_ASSERT(m_DockInformation); return *m_DockInformation; }
		const QDockWidget& dockInformation() const { Q_ASSERT(m_DockInformation); return *m_DockInformation; }

		QDockWidget& dockDevices() { Q_ASSERT(m_DockDevices); return *m_DockDevices; }
		const QDockWidget& dockDevices() const { Q_ASSERT(m_DockDevices); return *m_DockDevices; }

		QDockWidget& dockOperations() { Q_ASSERT(m_DockOperations); return *m_DockOperations; }
		const QDockWidget& dockOperations() const { Q_ASSERT(m_DockOperations); return *m_DockOperations; }

		QDockWidget& dockLog() { Q_ASSERT(m_DockLog); return *m_DockLog; }
		const QDockWidget& dockLog() const { Q_ASSERT(m_DockLog); return *m_DockLog; }

		QLabel& statusText() { Q_ASSERT(m_StatusText); return *m_StatusText; }
		const QLabel& statusText() const { Q_ASSERT(m_StatusText); return *m_StatusText; }

	protected slots:
		void on_m_ListDevices_selectionChanged(Device* d);

		void closeEvent(QCloseEvent*);
		void changeEvent(QEvent* event);

		void init();
		void updateDevices();
		void updateStatusBar();
		void updateSelection(const Partition* p);

	private:
		QLabel* m_StatusText;
		InfoPane* m_InfoPane;
		KActionCollection* m_ActionCollection;
};

#endif

