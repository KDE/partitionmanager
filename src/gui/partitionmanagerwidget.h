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

#if !defined(CENTRALWIDGET__H)

#define CENTRALWIDGET__H

#include "util/libpartitionmanagerexport.h"

#include "core/libparted.h"
#include "core/operationrunner.h"
#include "core/operationstack.h"

#include "ui_partitionmanagerwidgetbase.h"

#include <QWidget>

class QWidget;
class QLabel;
class PartWidget;
class KActionCollection;
class Device;
class ProgressDialog;

/** @brief The central widget for the application.

	@author vl@fidra.de
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT PartitionManagerWidget : public QWidget, Ui::PartitionManagerWidgetBase
{
	Q_OBJECT

	public:
		explicit PartitionManagerWidget(QWidget* parent, KActionCollection* coll = NULL);
		virtual ~PartitionManagerWidget();

	signals:
		void devicesChanged();
		void operationsChanged();
		void statusChanged();
		void selectionChanged(const Partition*);

	public slots:
		void setSelectedDevice(Device* d);

	public:
		void init(KActionCollection* coll);
		KActionCollection* actionCollection() const { return m_ActionCollection; }

		void clear();
		void clearSelection();
		void setPartitionTable(const PartitionTable* ptable);
		void setSelection(const Partition* p);
		void enableActions();

		Device* selectedDevice() { return m_SelectedDevice; }
		const Device* selectedDevice() const { return m_SelectedDevice; }

		Partition* selectedPartition();

		OperationStack::Devices& previewDevices() { return operationStack().previewDevices(); }
		const OperationStack::Devices& previewDevices() const { return operationStack().previewDevices(); }
		const OperationStack::Operations& operations() const { return operationStack().operations(); }

		void updatePartitions();

		Partition* clipboardPartition() { return m_ClipboardPartition; }
		const Partition* clipboardPartition() const { return m_ClipboardPartition; }
		void setClipboardPartition(Partition* p) { m_ClipboardPartition = p; }

		ProgressDialog& progressDialog() { Q_ASSERT(m_ProgressDialog); return *m_ProgressDialog; }
		const ProgressDialog& progressDialog() const { Q_ASSERT(m_ProgressDialog); return *m_ProgressDialog; }

		quint32 numPendingOperations();

	protected:
		void setupActions();
		void setupConnections();
		void showPartitionContextMenu(const QPoint& pos);
		void loadConfig();
		void saveConfig() const;
		bool showInsertDialog(Partition& insertPartition, qint64 sourceLength);

		PartTableWidget& partTableWidget() { Q_ASSERT(m_PartTableWidget); return *m_PartTableWidget; }
		const PartTableWidget& partTableWidget() const { Q_ASSERT(m_PartTableWidget); return *m_PartTableWidget; }

		QTreeWidget& treePartitions() { Q_ASSERT(m_TreePartitions); return *m_TreePartitions; }
		const QTreeWidget& treePartitions() const { Q_ASSERT(m_TreePartitions); return *m_TreePartitions; }

		LibParted& libParted() { return m_LibParted; }
		const LibParted& libParted() const { return m_LibParted; }

		OperationRunner& operationRunner() { return m_OperationRunner; }
		const OperationRunner& operationRunner() const { return m_OperationRunner; }

		OperationStack& operationStack() { return m_OperationStack; }
		const OperationStack& operationStack() const { return m_OperationStack; }

	protected slots:
		void on_m_TreePartitions_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
		void on_m_PartTableWidget_customContextMenuRequested(const QPoint& pos);
		void on_m_TreePartitions_customContextMenuRequested(const QPoint& pos);
		void on_m_TreePartitions_itemDoubleClicked(QTreeWidgetItem* item, int);
		void on_m_PartTableWidget_itemSelectionChanged(PartWidget* item);

		void scanDevices();

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
		void onFinished();

	private:
		LibParted m_LibParted;
		OperationStack m_OperationStack;
		OperationRunner m_OperationRunner;
		ProgressDialog* m_ProgressDialog;
		KActionCollection* m_ActionCollection;
		Device* m_SelectedDevice;
		Partition* m_ClipboardPartition;
};

#endif

