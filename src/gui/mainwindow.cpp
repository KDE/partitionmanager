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

#include "gui/mainwindow.h"
#include "gui/partwidget.h"
#include "gui/partpropsdialog.h"
#include "gui/resizedialog.h"
#include "gui/infopane.h"
#include "gui/newdialog.h"
#include "gui/filesystemsupportdialog.h"
#include "gui/progressdialog.h"
#include "gui/insertdialog.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/operationstack.h"
#include "core/partitiontable.h"
#include "core/operationrunner.h"

#include "fs/filesystemfactory.h"

#include "ops/deleteoperation.h"
#include "ops/resizeoperation.h"
#include "ops/newoperation.h"
#include "ops/copyoperation.h"
#include "ops/createpartitiontableoperation.h"
#include "ops/checkoperation.h"
#include "ops/backupoperation.h"
#include "ops/restoreoperation.h"
#include "ops/setfilesystemlabeloperation.h"
#include "ops/setpartflagsoperation.h"
#include "ops/createfilesystemoperation.h"

#include "util/globallog.h"
#include "util/capacity.h"
#include "util/report.h"

#include <kapplication.h>
#include <kglobalsettings.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kaboutdata.h>

#include <QLabel>
#include <QCloseEvent>
#include <QDateTime>
#include <QCursor>
#include <QHeaderView>

#include <config.h>

#include <unistd.h>

class PartitionTreeWidgetItem : public QTreeWidgetItem
{
	public:
		PartitionTreeWidgetItem(const Partition* p) : QTreeWidgetItem(), m_Partition(p) {}
		const Partition* partition() const { return m_Partition; }

	private:
		const Partition* m_Partition;
};

/** Creates a new MainWindow instance.
	@param parent the parent widget
	@param coll an action collection if used as a KPart
*/
MainWindow::MainWindow(QWidget* parent, KActionCollection* coll) :
	KXmlGuiWindow(parent),
	Ui::MainWindowBase(),
	m_LibParted(),
	m_OperationStack(),
	m_OperationRunner(operationStack()),
	m_StatusText(new QLabel(this)),
	m_InfoPane(new InfoPane(this)),
	m_ClipboardPartition(NULL),
	m_ProgressDialog(new ProgressDialog(this, operationRunner())),
	m_ActionCollection(coll)
{
	setupUi(this);

	FileSystemFactory::init();
	connect(GlobalLog::instance(), SIGNAL(newMessage(log::Level, const QString&)), SLOT(onNewLogMessage(log::Level, const QString&)));

	setupActions();
 	setupStatusBar();
	setupConnections();

	// If we were called with an action collection we're supposed to be a KPart, so don't
	// create the GUI in that case.
	if (coll != NULL)
		setupGUI(ToolBar | Keys | StatusBar | Save);
	else
		setupGUI(ToolBar | Keys | StatusBar | Save | Create);

	loadConfig();

	dockInformation().setWidget(&infoPane());
	treePartitions().header()->setStretchLastSection(false);

	scanDevices();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (progressDialog().isVisible())
	{
		event->ignore();
		return;
	}

	if (operationStack().size() > 0)
	{
		if (KMessageBox::warningContinueCancel(this,
			i18ncp("@info", "<para>Do you really want to quit the application?</para><para>There is still an operation pending.</para>",
    		"<para>Do you really want to quit the application?</para><para>There are still %1 operations pending.</para>", operationStack().size()),
			i18nc("@title:window", "Discard Pending Operations and Quit?"),
			KGuiItem(i18nc("@action:button", "&Quit <application>%1</application>", KGlobal::mainComponent().aboutData()->programName())),
			KStandardGuiItem::cancel(), "reallyQuit") == KMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}

	saveConfig();

	KXmlGuiWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent* event)
{
	if ((event->type() == QEvent::ActivationChange || event->type() == QEvent::WindowStateChange) && event->spontaneous() && isActiveWindow() && progressDialog().isVisible())
	{
		progressDialog().activateWindow();
		progressDialog().raise();
	}

	KXmlGuiWindow::changeEvent(event);
}

KActionCollection* MainWindow::actionCollection() const
{
	return m_ActionCollection != NULL ? m_ActionCollection : KXmlGuiWindow::actionCollection();
}

void MainWindow::setupActions()
{
	// File actions
	KStandardAction::quit(this, SLOT(close()), actionCollection());

	// Edit actions
	KAction* undoOperation = actionCollection()->addAction("undoOperation", this, SLOT(onUndoOperation()));
	undoOperation->setEnabled(false);
	undoOperation->setText(i18nc("@action:inmenu", "Undo"));
	undoOperation->setToolTip(i18nc("@info:tooltip", "Undo the last operation"));
	undoOperation->setStatusTip(i18nc("@info:status", "Remove the last operation from the list."));
	undoOperation->setShortcut(Qt::CTRL | Qt::Key_Z);
	undoOperation->setIcon(BarIcon("edit-undo"));

	KAction* clearAllOperations = actionCollection()->addAction("clearAllOperations", this, SLOT(onClearAllOperations()));
	clearAllOperations->setEnabled(false);
	clearAllOperations->setText(i18nc("@action:inmenu clear the list of operations", "Clear"));
	clearAllOperations->setToolTip(i18nc("@info:tooltip", "Clear all operations"));
	clearAllOperations->setStatusTip(i18nc("@info:status", "Empty the list of pending operations."));
	clearAllOperations->setIcon(BarIcon("dialog-cancel"));

	KAction* applyAllOperations = actionCollection()->addAction("applyAllOperations", this, SLOT(onApplyAllOperations()));
	applyAllOperations->setEnabled(false);
	applyAllOperations->setText(i18nc("@action:inmenu apply all operations", "Apply"));
	applyAllOperations->setToolTip(i18nc("@info:tooltip", "Apply all operations"));
	applyAllOperations->setStatusTip(i18nc("@info:status", "Apply the pending operations in the list."));
	applyAllOperations->setIcon(BarIcon("dialog-ok-apply"));

	// View actions
	actionCollection()->addAction("toggleDockDevices", dockDevices().toggleViewAction());
	actionCollection()->addAction("toggleDockOperations", dockOperations().toggleViewAction());
	actionCollection()->addAction("toggleDockInformation", dockInformation().toggleViewAction());
	actionCollection()->addAction("toggleDockLog", dockLog().toggleViewAction());

	KAction* fileSystemSupport = actionCollection()->addAction("fileSystemSupport", this, SLOT(onFileSystemSupport()));
	fileSystemSupport->setText(i18nc("@action:inmenu", "File System Support"));
	fileSystemSupport->setToolTip(i18nc("@info:tooltip", "View file system support information"));
	fileSystemSupport->setStatusTip(i18nc("@info:status", "Show information about supported file systems."));

	// Device actions
	KAction* refreshDevices = actionCollection()->addAction("refreshDevices", this, SLOT(onRefreshDevices()));
	refreshDevices->setText(i18nc("@action:inmenu refresh list of devices", "Refresh Devices"));
	refreshDevices->setToolTip(i18nc("@info:tooltip", "Refresh all devices"));
	refreshDevices->setStatusTip(i18nc("@info:status", "Renew the devices list."));
	refreshDevices->setShortcut(Qt::Key_F5);
	refreshDevices->setIcon(BarIcon("view-refresh"));

	KAction* createNewPartitionTable = actionCollection()->addAction("createNewPartitionTable", this, SLOT(onCreateNewPartitionTable()));
	createNewPartitionTable->setEnabled(false);
	createNewPartitionTable->setText(i18nc("@action:inmenu", "Create New Partition Table"));
	createNewPartitionTable->setToolTip(i18nc("@info:tooltip", "Create new partition table"));
	createNewPartitionTable->setStatusTip(i18nc("@info:status", "Create a new and empty partition table on a device."));
	createNewPartitionTable->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
	createNewPartitionTable->setIcon(BarIcon("edit-clear"));

	// Partition actions
	KAction* newPartition = actionCollection()->addAction("newPartition", this, SLOT(onNewPartition()));
	newPartition->setEnabled(false);
	newPartition->setText(i18nc("@action:inmenu create a new partition", "New"));
	newPartition->setToolTip(i18nc("@info:tooltip", "New partition"));
	newPartition->setStatusTip(i18nc("@info:status", "Create a new partition."));
	newPartition->setShortcut(Qt::CTRL | Qt::Key_N);
	newPartition->setIcon(BarIcon("document-new"));

	KAction* resizePartition = actionCollection()->addAction("resizePartition", this, SLOT(onResizePartition()));
	resizePartition->setEnabled(false);
	resizePartition->setText(i18nc("@action:inmenu", "Resize/Move"));
	resizePartition->setToolTip(i18nc("@info:tooltip", "Resize or move partition"));
	resizePartition->setStatusTip(i18nc("@info:status", "Shrink, grow or move an existing partition."));
	resizePartition->setShortcut(Qt::CTRL | Qt::Key_R);
	resizePartition->setIcon(BarIcon("arrow-right-double"));

	KAction* deletePartition = actionCollection()->addAction("deletePartition", this, SLOT(onDeletePartition()));
	deletePartition->setEnabled(false);
	deletePartition->setText(i18nc("@action:inmenu", "Delete"));
	deletePartition->setToolTip(i18nc("@info:tooltip", "Delete partition"));
	deletePartition->setStatusTip(i18nc("@info:status", "Delete a partition."));
	deletePartition->setShortcut(Qt::Key_Delete);
	deletePartition->setIcon(BarIcon("edit-delete-shred"));

	KAction* copyPartition = actionCollection()->addAction("copyPartition", this, SLOT(onCopyPartition()));
	copyPartition->setEnabled(false);
	copyPartition->setText(i18nc("@action:inmenu", "Copy"));
	copyPartition->setToolTip(i18nc("@info:tooltip", "Copy partition"));
	copyPartition->setStatusTip(i18nc("@info:status", "Copy an existing partition."));
	copyPartition->setShortcut(Qt::CTRL | Qt::Key_C);
	copyPartition->setIcon(BarIcon("edit-copy"));

	KAction* pastePartition = actionCollection()->addAction("pastePartition", this, SLOT(onPastePartition()));
	pastePartition->setEnabled(false);
	pastePartition->setText(i18nc("@action:inmenu", "Paste"));
	pastePartition->setToolTip(i18nc("@info:tooltip", "Paste partition"));
	pastePartition->setStatusTip(i18nc("@info:status", "Paste a copied partition."));
	pastePartition->setShortcut(Qt::CTRL | Qt::Key_V);
	pastePartition->setIcon(BarIcon("edit-paste"));

	KAction* mountPartition = actionCollection()->addAction("mountPartition", this, SLOT(onMountPartition()));
	mountPartition->setEnabled(false);
	mountPartition->setText(i18nc("@action:inmenu", "Mount"));
	mountPartition->setToolTip(i18nc("@info:tooltip", "Mount or unmount partition"));
	mountPartition->setStatusTip(i18nc("@info:status", "Mount or unmount a partition."));

	KAction* checkPartition = actionCollection()->addAction("checkPartition", this, SLOT(onCheckPartition()));
	checkPartition->setEnabled(false);
	checkPartition->setText(i18nc("@action:inmenu", "Check"));
	checkPartition->setToolTip(i18nc("@info:tooltip", "Check partition"));
	checkPartition->setStatusTip(i18nc("@info:status", "Check a filesystem on a partition for errors."));
	checkPartition->setIcon(BarIcon("flag"));

	KAction* propertiesPartition = actionCollection()->addAction("propertiesPartition", this, SLOT(onPropertiesPartition()));
	propertiesPartition->setEnabled(false);
	propertiesPartition->setText(i18nc("@action:inmenu", "Properties"));
	propertiesPartition->setToolTip(i18nc("@info:tooltip", "Show properties dialog"));
	propertiesPartition->setStatusTip(i18nc("@info:status", "View and modify partition properties (label, partition flags, etc.)"));
	propertiesPartition->setIcon(BarIcon("document-properties"));

	KAction* backup = actionCollection()->addAction("backupPartition", this, SLOT(onBackupPartition()));
	backup->setEnabled(false);
	backup->setText(i18nc("@action:inmenu", "Backup"));
	backup->setToolTip(i18nc("@info:tooltip", "Backup partition"));
	backup->setStatusTip(i18nc("@info:status", "Backup a partition to an image file."));
	backup->setIcon(BarIcon("document-export"));

	KAction* restore = actionCollection()->addAction("restorePartition", this, SLOT(onRestorePartition()));
	restore->setEnabled(false);
	restore->setText(i18nc("@action:inmenu", "Restore"));
	restore->setToolTip(i18nc("@info:tooltip", "Restore partition"));
	restore->setStatusTip(i18nc("@info:status", "Restore a partition from an image file."));
	restore->setIcon(BarIcon("document-import"));
}

void MainWindow::setupConnections()
{
	connect(&partTableWidget(), SIGNAL(itemActivated(const PartWidget*)), actionCollection()->action("propertiesPartition"), SLOT(trigger()));
	connect(&progressDialog(), SIGNAL(finished(int)), SLOT(onFinished()));
	connect(this, SIGNAL(devicesChanged()), SLOT(scanDevices()));
}

void MainWindow::setupStatusBar()
{
	statusBar()->addWidget(&statusText());
	updateStatusBar();
}

void MainWindow::loadConfig()
{
	QList<int> colWidths = Config::treePartitionColumnWidths();

	if (!colWidths.isEmpty() && colWidths[0] != -1)
		for (int i = 0; i < colWidths.size(); i++)
			treePartitions().setColumnWidth(i, colWidths[i]);

	bool firstRun = Config::firstRun();

	if (firstRun)
	{
		dockLog().setVisible(false);
		dockInformation().setVisible(false);
		toolBar("deviceToolBar")->setVisible(false);
	}
}

void MainWindow::saveConfig() const
{
	QList<int> colWidths;
	for(int i = 0; i < treePartitions().columnCount(); i++)
		colWidths.append(treePartitions().columnWidth(i));
	Config::setTreePartitionColumnWidths(colWidths);

	Config::setFirstRun(false);

	Config::self()->writeConfig();
}

void MainWindow::updateStatusBar()
{
	statusText().setText(i18ncp("@info:status", "One pending operation", "%1 pending operations", operationStack().size()));
}

void MainWindow::scanDevices()
{
	log() << i18nc("@info/plain", "Rescan devices...");

	KApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	const qint64 selectedDeviceIdx = selectedDevice() ? listDevices().currentRow() : -1;

	listDevices().clearSelection();
	setClipboardPartition(NULL);
	partTableWidget().clear();

	libParted().scanDevices(operationStack());

	setupDevicesList();
	updatePartitions();
	updateOperations();
	updateStatusBar();

	if (selectedDeviceIdx > -1 && selectedDeviceIdx < listDevices().count())
		listDevices().setCurrentRow(selectedDeviceIdx);

	log() << i18nc("@info/plain", "Rescan finished.");
	KApplication::restoreOverrideCursor();
}

void MainWindow::setupDevicesList()
{
	listDevices().clear();

	foreach(const Device* d, operationStack().previewDevices())
	{
		const QString shortText = d->deviceNode() + " (" + Capacity(*d).toString() + ')';
		const QString longText = d->deviceNode() + " (" + Capacity(*d).toString() + ", " + d->name() + ')';
		QListWidgetItem* item = new QListWidgetItem(SmallIcon("drive-harddisk"), shortText);
		item->setToolTip(longText);
		listDevices().addItem(item);
	}

	enableActions();
}

void MainWindow::enableActions()
{
	actionCollection()->action("createNewPartitionTable")->setEnabled(CreatePartitionTableOperation::canCreate(selectedDevice()));

	const Partition* part = selectedPartition();

	const bool readOnly = selectedDevice() == NULL || selectedDevice()->partitionTable().isReadOnly();

	actionCollection()->action("newPartition")->setEnabled(!readOnly && NewOperation::canCreateNew(part));
	const bool canResize = ResizeOperation::canGrow(part) || ResizeOperation::canShrink(part) || ResizeOperation::canMove(part);
	actionCollection()->action("resizePartition")->setEnabled(!readOnly && canResize);
	actionCollection()->action("copyPartition")->setEnabled(CopyOperation::canCopy(part));
	actionCollection()->action("deletePartition")->setEnabled(!readOnly && DeleteOperation::canDelete(part));
	actionCollection()->action("pastePartition")->setEnabled(!readOnly && CopyOperation::canPaste(part, clipboardPartition()));
	actionCollection()->action("propertiesPartition")->setEnabled(part != NULL);

	actionCollection()->action("mountPartition")->setEnabled(part && (part->canMount() || part->canUnmount()));

	if (part != NULL)
		actionCollection()->action("mountPartition")->setText(part->isMounted() ? part->fileSystem().unmountTitle() : part->fileSystem().mountTitle() );

	actionCollection()->action("checkPartition")->setEnabled(!readOnly && CheckOperation::canCheck(part));

	actionCollection()->action("undoOperation")->setEnabled(operationStack().size() > 0);
	actionCollection()->action("clearAllOperations")->setEnabled(operationStack().size() > 0);
	actionCollection()->action("applyAllOperations")->setEnabled(operationStack().size() > 0 && geteuid() == 0);

	actionCollection()->action("backupPartition")->setEnabled(BackupOperation::canBackup(part));
	actionCollection()->action("restorePartition")->setEnabled(RestoreOperation::canRestore(part));
}

void MainWindow::on_m_ListDevices_itemClicked()
{
	treePartitions().setCurrentItem(NULL);
	enableActions();
	updatePartitions();
}

void MainWindow::updateDevices()
{
	enableActions();
	updatePartitions();

	if (selectedDevice())
		infoPane().showDevice(*selectedDevice());
	else
		infoPane().clear();
}

void MainWindow::on_m_ListDevices_itemSelectionChanged()
{
	updateDevices();
}

void MainWindow::updateOperations()
{
	listOperations().clear();

	foreach (const Operation* op, operationStack().operations())
	{
		QListWidgetItem* item = new QListWidgetItem(SmallIcon(op->iconName()), op->description());
		item->setToolTip(op->description());
		listOperations().addItem(item);
	}

	listOperations().scrollToBottom();
}

static QTreeWidgetItem* createTreeWidgetItem(const Partition& p)
{
	QTreeWidgetItem* item = new PartitionTreeWidgetItem(&p);

	item->setText(0, p.deviceNode());
	item->setText(1, p.fileSystem().name());
	item->setText(2, p.mountPoints().join(", "));
	if (p.isMounted())
		item->setIcon(2, SmallIcon("object-locked"));
	item->setText(3, p.fileSystem().label());
	item->setText(4, Capacity(p).toString());
	item->setText(5, Capacity(p, Capacity::Used).toString());
	item->setText(6, PartitionTable::flagNames(p.activeFlags()).join(", "));

	return item;
}

void MainWindow::updateWindowTitle()
{
	QString title;

	if (selectedDevice())
		title = selectedDevice()->deviceNode() + " - ";

	title += KGlobal::mainComponent().aboutData()->programName() + ' ' + KGlobal::mainComponent().aboutData()->version();

	setWindowTitle(title);
}

void MainWindow::updatePartitions()
{
	treePartitions().clear();
	partTableWidget().clear();
	updateWindowTitle();

	if (selectedDevice() == NULL)
		return;

	partTableWidget().setPartitionTable(&selectedDevice()->partitionTable());

	QTreeWidgetItem* deviceItem = new QTreeWidgetItem();
	deviceItem->setText(0, selectedDevice()->name());
	deviceItem->setIcon(0, SmallIcon("drive-harddisk"));
	treePartitions().addTopLevelItem(deviceItem);

	foreach(const Partition* p, selectedDevice()->partitionTable().children())
	{
		QTreeWidgetItem* item = createTreeWidgetItem(*p);

		foreach(const Partition* child, p->children())
		{
			QTreeWidgetItem* childItem = createTreeWidgetItem(*child);
			item->addChild(childItem);
		}

		deviceItem->addChild(item);
		item->setExpanded(true);
	}

	treePartitions().setFirstItemColumnSpanned(deviceItem, true);
	deviceItem->setExpanded(true);
	deviceItem->setFlags(Qt::ItemIsEnabled);

	partTableWidget().update();
}

void MainWindow::on_m_TreePartitions_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem*)
{
	if (current)
	{
		const PartitionTreeWidgetItem* ptwItem = dynamic_cast<PartitionTreeWidgetItem*>(current);
		partTableWidget().setActivePartition(ptwItem ? ptwItem->partition() : NULL);
	}
	else
		partTableWidget().setActiveWidget(NULL);

	updateWindowTitle();
}

void MainWindow::on_m_TreePartitions_itemDoubleClicked(QTreeWidgetItem* item, int)
{
	// if the activated item is the device item, don't do anything
	if (item == treePartitions().topLevelItem(0))
		return;

	actionCollection()->action("propertiesPartition")->trigger();
}

Partition* MainWindow::selectedPartition()
{
	if (selectedDevice() == NULL || partTableWidget().activeWidget() == NULL || partTableWidget().activeWidget()->partition() == NULL)
		return NULL;

	// The active partition we get from PartTableWidget is const; we need non-const.
	// So take the first sector and find the partition in the selected device's
	// partition table.
	const Partition* activePartition = partTableWidget().activeWidget()->partition();
	PartitionTable& ptable = selectedDevice()->partitionTable();

	return ptable.findPartitionBySector(activePartition->firstSector(), PartitionRole(PartitionRole::Any));
}

Device* MainWindow::selectedDevice()
{
	if (listDevices().selectedItems().size() != 1)
		return NULL;

	int idx = listDevices().row(listDevices().selectedItems()[0]);

	if (idx < 0 || idx >= operationStack().previewDevices().size())
		return NULL;

	return operationStack().previewDevices()[idx];
}

void MainWindow::on_m_PartTableWidget_itemSelectionChanged(PartWidget* item)
{
	enableActions();

	if (item == NULL)
	{
		if (selectedDevice())
			infoPane().showDevice(*selectedDevice());
		else
			infoPane().clear();

		treePartitions().setCurrentItem(NULL);
		return;
	}

	const Partition* p = item->partition();

	Q_ASSERT(p);

	QList<QTreeWidgetItem*> findResult = treePartitions().findItems(p->deviceNode(), Qt::MatchFixedString | Qt::MatchRecursive, 0);

	for (int idx = 0; idx < findResult.size(); idx++)
	{
		const PartitionTreeWidgetItem* ptwItem = dynamic_cast<PartitionTreeWidgetItem*>(findResult[idx]);

		if (ptwItem && ptwItem->partition() == p)
		{
			treePartitions().setCurrentItem(findResult[idx]);
			break;
		}
	}

	infoPane().showPartition(*p);
}

void MainWindow::on_m_PartTableWidget_customContextMenuRequested(const QPoint& pos)
{
	showPartitionContextMenu(partTableWidget().mapToGlobal(pos));
}

void MainWindow::on_m_TreePartitions_customContextMenuRequested(const QPoint& pos)
{
	showPartitionContextMenu(treePartitions().viewport()->mapToGlobal(pos));
}

void MainWindow::showPartitionContextMenu(const QPoint& pos)
{
	if (selectedPartition() == NULL)
		return;

	KMenu partitionMenu;

	partitionMenu.addAction(actionCollection()->action("newPartition"));
	partitionMenu.addAction(actionCollection()->action("resizePartition"));
	partitionMenu.addAction(actionCollection()->action("deletePartition"));
	partitionMenu.addSeparator();
	partitionMenu.addAction(actionCollection()->action("copyPartition"));
	partitionMenu.addAction(actionCollection()->action("pastePartition"));
	partitionMenu.addSeparator();
	partitionMenu.addAction(actionCollection()->action("mountPartition"));
	partitionMenu.addSeparator();
	partitionMenu.addAction(actionCollection()->action("checkPartition"));
	partitionMenu.addSeparator();
	partitionMenu.addAction(actionCollection()->action("propertiesPartition"));

	partitionMenu.exec(pos);
	statusBar()->clearMessage();
}

void MainWindow::on_m_ListDevices_customContextMenuRequested(const QPoint& pos)
{
	on_m_ListDevices_itemClicked();

	KMenu deviceMenu;

	deviceMenu.addAction(actionCollection()->action("createNewPartitionTable"));

	deviceMenu.exec(listDevices().viewport()->mapToGlobal(pos));
	statusBar()->clearMessage();
}

void MainWindow::onPropertiesPartition()
{
	if (selectedPartition())
	{
		Q_ASSERT(selectedDevice());

		PartPropsDialog dlg(this, *selectedDevice(), *selectedPartition());

		if (dlg.exec() == KDialog::Accepted)
		{
			if (dlg.newFileSystemType() != selectedPartition()->fileSystem().type() || dlg.forceRecreate())
				operationStack().push(new CreateFileSystemOperation(*selectedDevice(), *selectedPartition(), dlg.newFileSystemType()));

			if (dlg.newLabel() != selectedPartition()->fileSystem().label())
				operationStack().push(new SetFileSystemLabelOperation(*selectedPartition(), dlg.newLabel()));

			if (dlg.newFlags() != selectedPartition()->activeFlags())
				operationStack().push(new SetPartFlagsOperation(*selectedDevice(), *selectedPartition(), dlg.newFlags()));

			updatePartitions();
			updateOperations();
			updateStatusBar();
		}
	}
}

void MainWindow::onMountPartition()
{
	Partition* p = selectedPartition();
	Report report(NULL);

	if (p && p->canMount())
	{
		if (!p->mount(report))
			KMessageBox::detailedSorry(this, i18nc("@info", "The file system on partition <filename>%1</filename> could not be mounted.", p->deviceNode()), QString("<pre>%1</pre>").arg(report.toText()), i18nc("@title:window", "Could Not Mount File System."));
	}
	else if (p && p->canUnmount())
	{
		if (!p->unmount(report))
			KMessageBox::detailedSorry(this, i18nc("@info", "The file system on partition <filename>%1</filename> could not be unmounted.", p->deviceNode()), QString("<pre>%1</pre>").arg(report.toText()), i18nc("@title:window", "Could Not Unmount File System."));
	}

	if (p->roles().has(PartitionRole::Logical))
	{
		Partition* parent = dynamic_cast<Partition*>(p->parent());

		Q_ASSERT(parent);

		if (parent != NULL)
			parent->checkChildrenMounted();
		else
			kWarning() << "parent is null";
	}

	enableActions();
	updatePartitions();
}

void MainWindow::onFinished()
{
	partTableWidget().setUpdatesEnabled(true);

#if !defined(NDEBUG) && 0
	operationStack().clearOperations();
	updatePartitions();
	updateOperations();
	enableActions();
#else
	devicesChanged();
#endif
}

static bool checkTooManyPartitions(QWidget* parent, const Device& d, const Partition& p)
{
	if (p.roles().has(PartitionRole::Unallocated) && d.partitionTable().numPrimaries() >= d.partitionTable().maxPrimaries() && !p.roles().has(PartitionRole::Logical))
	{
		KMessageBox::sorry(parent, i18nc("@info",
			"<para>There are already %1 primary partitions on this device. This is the maximum number its partition table can handle.</para>"
			"<para>You cannot create, paste or restore a primary partition on it before you delete an existing one.</para>",
			d.partitionTable().numPrimaries()), i18nc("@title:window", "Too Many Primary Partitions."));
		return true;
	}

	return false;
}

void MainWindow::onNewPartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	if (checkTooManyPartitions(this, *selectedDevice(), *selectedPartition()))
		return;

	Partition* newPartition = NewOperation::createNew(*selectedPartition());

	NewDialog dlg(this, *selectedDevice(), *newPartition, selectedDevice()->partitionTable().childRoles(*selectedPartition()));
	if (dlg.exec() == KDialog::Accepted)
	{
 		PartitionTable::snap(*selectedDevice(), *newPartition);
		operationStack().push(new NewOperation(*selectedDevice(), newPartition));
		updatePartitions();
		updateStatusBar();
		updateOperations();
	}
	else
		delete newPartition;
}

void MainWindow::onDeletePartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	if (selectedPartition()->roles().has(PartitionRole::Logical))
	{
		Q_ASSERT(selectedPartition()->parent());

		if (selectedPartition()->parent() == NULL)
		{
			kWarning() << "parent of selected partition is null.";
			return;
		}

		if (selectedPartition()->parent()->highestMountedChild() > selectedPartition()->number())
		{
			KMessageBox::sorry(this,
					i18nc("@info",
						"<para>The partition <filename>%1</filename> cannot currently be deleted because one or more partitions with higher logical numbers are still mounted.</para>"
						"<para>Please unmount all partitions with higher logical numbers than %2 first.</para>",
						selectedPartition()->deviceNode(), selectedPartition()->number()),
					i18nc("@title:window", "Cannot Delete Partition."));

			return;
		}
	}

	if (clipboardPartition() == selectedPartition())
	{
		if (KMessageBox::warningContinueCancel(this,
				i18nc("@info",
					"Do you really want to delete the partition that is currently in the clipboard? "
					"It will no longer be available for pasting after it has been deleted."),
				i18nc("@title:window", "Really Delete Partition in the Clipboard?"),
				KGuiItem(i18nc("@action:button", "&Delete It")),
				KStandardGuiItem::cancel(), "reallyDeleteClipboardPartition") == KMessageBox::Cancel)
			return;

		setClipboardPartition(NULL);
	}

	operationStack().push(new DeleteOperation(*selectedDevice(), selectedPartition()));
	updatePartitions();
	updateStatusBar();
	updateOperations();
}

void MainWindow::onResizePartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	const qint64 freeBefore = selectedDevice()->partitionTable().freeSectorsBefore(*selectedPartition());
	const qint64 freeAfter = selectedDevice()->partitionTable().freeSectorsAfter(*selectedPartition());

	Partition resizedPartition(*selectedPartition());
	ResizeDialog dlg(this, *selectedDevice(), resizedPartition, freeBefore, freeAfter);

	if (dlg.exec() == KDialog::Accepted && dlg.isModified())
	{
 		PartitionTable::snap(*selectedDevice(), resizedPartition, selectedPartition());

		if (resizedPartition.firstSector() == selectedPartition()->firstSector() && resizedPartition.lastSector() == selectedPartition()->lastSector())
			log(log::information) << i18nc("@info/plain", "Partition <filename>%1</filename> has the same position and size after resize/move. Ignoring operation.", selectedPartition()->deviceNode());
		else
		{
			operationStack().push(new ResizeOperation(*selectedDevice(), *selectedPartition(), resizedPartition.firstSector(), resizedPartition.lastSector()));

			updatePartitions();
			updateStatusBar();
			updateOperations();
		}
	}
}

void MainWindow::onCopyPartition()
{
	Q_ASSERT(selectedPartition());

	if (selectedPartition() == NULL)
	{
		kWarning() << "selected partition: " << selectedPartition();
		return;
	}

	setClipboardPartition(selectedPartition());
	log() << i18nc("@info/plain", "Partition <filename>%1</filename> has been copied to the clipboard.", selectedPartition()->deviceNode());

	enableActions();
}

void MainWindow::onPastePartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	if (clipboardPartition() == NULL)
	{
		kWarning() << "no partition in the clipboard.";
		return;
	}

	if (checkTooManyPartitions(this, *selectedDevice(), *selectedPartition()))
		return;

	Device* dSource = operationStack().findDeviceForPartition(clipboardPartition());

	Q_ASSERT(dSource);

	if (dSource == NULL)
	{
		kWarning() << "source partition is null.";
		return;
	}

	Partition* copiedPartition = CopyOperation::createCopy(*selectedPartition(), *clipboardPartition());

	if (showInsertDialog(*copiedPartition, clipboardPartition()->length()))
	{
		operationStack().push(new CopyOperation(*selectedDevice(), copiedPartition, *dSource, clipboardPartition()));
		updatePartitions();
		updateStatusBar();
		updateOperations();
	}
	else
		delete copiedPartition;
}

bool MainWindow::showInsertDialog(Partition& insertPartition, qint64 sourceLength)
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return false;
	}

	const bool overwrite = !selectedPartition()->roles().has(PartitionRole::Unallocated);

	// Make sure the inserted partition has the right parent and logical or primary set. Only then
	// can Device::snap() work correctly.
	selectedPartition()->parent()->reparent(insertPartition);

	if (!overwrite)
	{
		InsertDialog dlg(this, *selectedDevice(), insertPartition, *selectedPartition());
		if (dlg.exec() != KDialog::Accepted)
			return false;
		PartitionTable::snap(*selectedDevice(), insertPartition, selectedPartition());
	}

	if (insertPartition.length() < sourceLength)
	{
		if (overwrite)
			KMessageBox::error(this, i18nc("@info",
				"<para>The selected partition is not large enough to hold the source partition or the backup file.</para>"
				"<para>Pick another target or resize this partition so it is as large as the source.</para>"), i18nc("@title:window", "Target Not Large Enough"));
		else
			KMessageBox::sorry(this, i18nc("@info",
				"<para>It is not possible to create the target partition large enough to hold the source.</para>"
				"<para>This may happen if not all partitions on a device start and end on cylinder boundaries "
				"or when copying a primary partition into an extended partition.</para>"),
				i18nc("@title:window", "Cannot Create Target Partition."));
		return false;
	}

	return true;
}

void MainWindow::onCreateNewPartitionTable()
{
	Q_ASSERT(selectedDevice());

	if (selectedDevice() == NULL)
	{
		kWarning() << "selected device is null.";
		return;
	}

	if (KMessageBox::warningContinueCancel(this,
		i18nc("@info",
			"<para>Do you really want to create a new partition table on the following device?</para>"
			"<para><list><item><filename>%1</filename> (%2)</item></list></para>"
			"<para><warning>This will destroy all data on the device.</warning></para>", selectedDevice()->deviceNode(), selectedDevice()->name()),
		i18nc("@title:window", "Destroy All Data on Device?"),
		KGuiItem(i18nc("@action:button", "&Create New Partition Table")),
		KStandardGuiItem::cancel()) == KMessageBox::Continue)
	{
		operationStack().push(new CreatePartitionTableOperation(*selectedDevice()));

		updateDevices();
		updatePartitions();
		updateStatusBar();
		updateOperations();
		enableActions();
	}
}

void MainWindow::onRefreshDevices()
{
	if (operationStack().size() == 0 || KMessageBox::warningContinueCancel(this,
		i18nc("@info",
			"<para>Do you really want to rescan the devices?</para>"
			"<para><warning>This will also clear the list of pending operations.</warning></para>"),
		i18nc("@title:window", "Really Rescan the Devices?"),
		KGuiItem(i18nc("@action:button", "&Rescan Devices")),
		KStandardGuiItem::cancel(), "reallyRescanDevices") == KMessageBox::Continue)
	{
		scanDevices();
	}
}

void MainWindow::onUndoOperation()
{
	log() << i18nc("@info/plain", "Undoing operation: %1", operationStack().operations().last()->description());
	operationStack().pop();

	updateDevices();
	updatePartitions();
	updateOperations();
	updateStatusBar();
	enableActions();
}

void MainWindow::onClearAllOperations()
{
	if (KMessageBox::warningContinueCancel(this,
		i18nc("@info", "Do you really want to clear the list of pending operations?"),
		i18nc("@title:window", "Clear Pending Operations?"),
		KGuiItem(i18nc("@action:button", "&Clear Pending Operations")),
		KStandardGuiItem::cancel(), "reallyClearPendingOperations") == KMessageBox::Continue)
	{
		log() << i18nc("@info/plain", "Clearing the list of pending operations.");
		operationStack().clearOperations();

		updateDevices();
		updatePartitions();
		updateOperations();
		updateStatusBar();
		enableActions();
	}
}

void MainWindow::onApplyAllOperations()
{
	QStringList opList;

	foreach (const Operation* op, operationStack().operations())
		opList.append(op->description());

	if (KMessageBox::warningContinueCancelList(this,
		i18nc("@info",
			"<para>Do you really want to apply the pending operations listed below?</para>"
			"<para><warning>This will permanently modify your disks.</warning></para>"),
		opList, i18nc("@title:window", "Apply Pending Operations?"),
		KGuiItem(i18nc("@action:button", "&Apply Pending Operations")),
		KStandardGuiItem::cancel()) == KMessageBox::Continue)
	{
		log() << i18nc("@info/plain", "Applying operations...");

		progressDialog().show();

		operationRunner().setReport(&progressDialog().report());

		partTableWidget().setUpdatesEnabled(false);

		// Undo all operations so the runner has a defined starting point
		for (int i = operationStack().operations().size() - 1; i >= 0; i--)
		{
			operationStack().operations()[i]->undo();
			operationStack().operations()[i]->setStatus(Operation::StatusNone);
		}

		updatePartitions();

 		operationRunner().start();
	}
}

void MainWindow::onCheckPartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	operationStack().push(new CheckOperation(*selectedDevice(), *selectedPartition()));

	updatePartitions();
	updateStatusBar();
	updateOperations();
}

void MainWindow::onBackupPartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	QString fileName = KFileDialog::getSaveFileName(KUrl("kfiledialog://backupPartition"));
// 	QString fileName = "/tmp/backuptest.img";

	if (fileName.isEmpty())
		return;

	if (!QFile::exists(fileName) || KMessageBox::warningContinueCancel(this, i18nc("@info", "Do you want to overwrite the existing file <filename>%1</filename>?", fileName), i18nc("@title:window", "Overwrite Existing File?"), KGuiItem(i18nc("@action:button", "&Overwrite File")), KStandardGuiItem::cancel()) == KMessageBox::Continue)
	{
		operationStack().push(new BackupOperation(*selectedDevice(), *selectedPartition(), fileName));
		updatePartitions();
		updateStatusBar();
		updateOperations();
	}
}

void MainWindow::onRestorePartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	if (checkTooManyPartitions(this, *selectedDevice(), *selectedPartition()))
		return;

	QString fileName = KFileDialog::getOpenFileName(KUrl("kfiledialog://backupPartition"));
// 	QString fileName = "/tmp/backuptest.img";

	if (!fileName.isEmpty() && QFile::exists(fileName))
	{
		Partition* restorePartition = RestoreOperation::createRestorePartition(*selectedDevice(), *selectedPartition()->parent(), selectedPartition()->firstSector(), fileName);

		if (restorePartition->length() > selectedPartition()->length())
		{
			KMessageBox::error(this, i18nc("@info", "The file system in the image file <filename>%1</filename> is too large to be restored to the selected partition.", fileName), i18nc("@title:window", "Not Enough Space to Restore File System."));
			delete restorePartition;
			return;
		}

		if (showInsertDialog(*restorePartition, restorePartition->length()))
		{
			operationStack().push(new RestoreOperation(*selectedDevice(), restorePartition, fileName));

			updatePartitions();
			updateStatusBar();
			updateOperations();
		}
		else
			delete restorePartition;
	}
}

void MainWindow::onFileSystemSupport()
{
	FileSystemSupportDialog dlg(this);
	dlg.exec();
}

void MainWindow::onNewLogMessage(log::Level logLevel, const QString& s)
{
	static const char* icons[] =
	{
		"tools-report-bug",
		"dialog-information",
		"dialog-warning",
		"dialog-error"
	};

	kDebug() << s;

	QTreeWidgetItem* item = new QTreeWidgetItem();

	item->setIcon(0, SmallIcon(icons[logLevel]));
	item->setText(0, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	item->setText(1, s);

	treeLog().addTopLevelItem(item);

	for (int i = 0; i < treeLog().model()->columnCount(); i++)
		treeLog().resizeColumnToContents(i);

	treeLog().scrollToBottom();
}
