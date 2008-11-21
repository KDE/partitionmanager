/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
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

#include "ui_mainwindowbase.h"

#include "core/libparted.h"
#include "core/operationrunner.h"
#include "core/operationstack.h"

#include "util/globallog.h"

#include <kxmlguiwindow.h>

class QWidget;
class QLabel;
class PartWidget;
class InfoPane;
class QCloseEvent;
class QEvent;
class Device;
class ProgressDialog;

/** @brief The application's main window.

	@author vl@fidra.de
*/
class MainWindow : public KXmlGuiWindow, public Ui::MainWindowBase
{
	Q_OBJECT

	public:
		MainWindow(QWidget* parent = NULL);

	signals:
		void devicesChanged();

	protected:
		void setupActions();
		void setupConnections();
		void setupStatusBar();
		void setupDevicesList();
		void loadConfig();
		void saveConfig() const;
		void updateWindowTitle();
		void updateStatusBar();
		void updateOperations();
		void enableActions();
		void showPartitionContextMenu(const QPoint& pos);
		void updateDevices();
		void updatePartitions();

		bool showInsertDialog(Partition& insertPartition, qint64 sourceLength);

		Device* selectedDevice();
		Partition* selectedPartition();
		
		InfoPane& infoPane() { Q_ASSERT(m_InfoPane); return *m_InfoPane; }
		
		PartTableWidget& partTableWidget() { Q_ASSERT(m_PartTableWidget); return *m_PartTableWidget; }
		const PartTableWidget& partTableWidget() const { Q_ASSERT(m_PartTableWidget); return *m_PartTableWidget; }

		QListWidget& listDevices() { Q_ASSERT(m_ListDevices); return *m_ListDevices; }
		const QListWidget& listDevices() const { Q_ASSERT(m_ListDevices); return *m_ListDevices; }

		QListWidget& listOperations() { Q_ASSERT(m_ListOperations); return *m_ListOperations; }
		const QListWidget& listOperations() const { Q_ASSERT(m_ListOperations); return *m_ListOperations; }
		
		QTreeWidget& treePartitions() { Q_ASSERT(m_TreePartitions); return *m_TreePartitions; }
		const QTreeWidget& treePartitions() const { Q_ASSERT(m_TreePartitions); return *m_TreePartitions; }
		
		QDockWidget& dockInformation() { Q_ASSERT(m_DockInformation); return *m_DockInformation; }
		const QDockWidget& dockInformation() const { Q_ASSERT(m_DockInformation); return *m_DockInformation; }

		QDockWidget& dockDevices() { Q_ASSERT(m_DockDevices); return *m_DockDevices; }
		const QDockWidget& dockDevices() const { Q_ASSERT(m_DockDevices); return *m_DockDevices; }

		QDockWidget& dockOperations() { Q_ASSERT(m_DockOperations); return *m_DockOperations; }
		const QDockWidget& dockOperations() const { Q_ASSERT(m_DockOperations); return *m_DockOperations; }

		QDockWidget& dockLog() { Q_ASSERT(m_DockLog); return *m_DockLog; }
		const QDockWidget& dockLog() const { Q_ASSERT(m_DockLog); return *m_DockLog; }

		QTreeWidget& treeLog() { Q_ASSERT(m_TreeLog); return *m_TreeLog; }
		const QTreeWidget& treeLog() const { Q_ASSERT(m_TreeLog); return *m_TreeLog; }
		
		Partition* clipboardPartition() { return m_ClipboardPartition; }
		const Partition* clipboardPartition() const { return m_ClipboardPartition; }
		void setClipboardPartition(Partition* p) { m_ClipboardPartition = p; }

		LibParted& libParted() { return m_LibParted; }
		const LibParted& libParted() const { return m_LibParted; }
		
		ProgressDialog& progressDialog() { Q_ASSERT(m_ProgressDialog); return *m_ProgressDialog; }
		
		OperationRunner& operationRunner() { return m_OperationRunner; }
		const OperationRunner& operationRunner() const { return m_OperationRunner; }

		OperationStack& operationStack() { return m_OperationStack; }
		const OperationStack& operationStack() const { return m_OperationStack; }

		QLabel& statusText() { Q_ASSERT(m_StatusText); return *m_StatusText; }
		const QLabel& statusText() const { Q_ASSERT(m_StatusText); return *m_StatusText; }

	protected slots:
		void on_m_ListDevices_itemSelectionChanged();
		void on_m_ListDevices_customContextMenuRequested(const QPoint& pos);
		void on_m_TreePartitions_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
		void on_m_PartTableWidget_customContextMenuRequested(const QPoint& pos);
		void on_m_TreePartitions_customContextMenuRequested(const QPoint& pos);
		void on_m_TreePartitions_itemDoubleClicked(QTreeWidgetItem* item, int);
		void on_m_PartTableWidget_itemSelectionChanged(PartWidget* item);
		void on_m_ListDevices_itemClicked();

		void onPropertiesPartition();
		void onMountPartition();
		void onNewPartition();
		void onDeletePartition();
		void onResizePartition();
		void onCopyPartition();
		void onPastePartition();
		void onCheckPartition();
		void onCreateNewPartitionTable();
		void onRefreshDevices();
		void onUndoOperation();
		void onClearAllOperations();
		void onApplyAllOperations();
		void onFileSystemSupport();
		void onBackupPartition();
		void onRestorePartition();

		void closeEvent(QCloseEvent*);
		void changeEvent(QEvent* event);

		void onNewLogMessage(log::Level logLevel, const QString& s);
		void onFinished();
		void scanDevices();
		
	private:
		LibParted m_LibParted;
		OperationStack m_OperationStack;
		OperationRunner m_OperationRunner;
		QLabel* m_StatusText;
		InfoPane* m_InfoPane;
		Partition* m_ClipboardPartition;
		ProgressDialog* m_ProgressDialog;
};

#endif

