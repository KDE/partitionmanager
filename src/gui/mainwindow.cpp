/***************************************************************************
 *   Copyright (C) 2008,2009,2010 by Volker Lanz <vl@fidra.de>             *
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
#include "gui/infopane.h"
#include "gui/applyprogressdialog.h"
#include "gui/scanprogressdialog.h"
#include "gui/createpartitiontabledialog.h"
#include "gui/filesystemsupportdialog.h"
#include "gui/configureoptionsdialog.h"
#include "gui/devicepropsdialog.h"

#include "core/device.h"
#include "core/partition.h"

#include "ops/operation.h"
#include "ops/createpartitiontableoperation.h"
#include "ops/resizeoperation.h"
#include "ops/copyoperation.h"
#include "ops/deleteoperation.h"
#include "ops/newoperation.h"
#include "ops/backupoperation.h"
#include "ops/restoreoperation.h"
#include "ops/checkoperation.h"

#include "fs/filesystem.h"

#include <kstandardaction.h>
#include <kactioncollection.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kstandardguiitem.h>
#include <kaction.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kxmlguifactory.h>

#include <QCloseEvent>
#include <QReadLocker>
#include <QPointer>

#include <config.h>

#include <unistd.h>

/** Creates a new MainWindow instance.
	@param parent the parent widget
	@param coll an action collection if used as a KPart
*/
MainWindow::MainWindow(QWidget* parent, KActionCollection* coll) :
	KXmlGuiWindow(parent),
	Ui::MainWindowBase(),
	m_ActionCollection(coll),
	m_OperationStack(new OperationStack(this)),
	m_OperationRunner(new OperationRunner(this, operationStack())),
	m_DeviceScanner(new DeviceScanner(this, operationStack())),
	m_ApplyProgressDialog(new ApplyProgressDialog(this, operationRunner())),
	m_ScanProgressDialog(new ScanProgressDialog(this)),
	m_StatusText(new QLabel(this)),
	m_InfoPane(new InfoPane(this))
{
	setupObjectNames();
	setupUi(this);
	init();
}

void MainWindow::setupObjectNames()
{
	m_OperationStack->setObjectName("m_OperationStack");
	m_OperationRunner->setObjectName("m_OperationRunner");
	m_DeviceScanner->setObjectName("m_DeviceScanner");
	m_ApplyProgressDialog->setObjectName("m_ApplyProgressDialog");
	m_ScanProgressDialog->setObjectName("m_ScanProgressDialog");
}

void MainWindow::init()
{
	treeLog().init(actionCollection(), &pmWidget());

	connect(GlobalLog::instance(), SIGNAL(newMessage(Log::Level, const QString&)), &treeLog(), SLOT(onNewLogMessage(Log::Level, const QString&)));

	setupActions();
	setupStatusBar();
	setupConnections();

	listDevices().setActionCollection(actionCollection());
	listOperations().setActionCollection(actionCollection());
	pmWidget().init(&operationStack(), "partitionmanagerrc");

	if (isKPart())
	{
		setupGUI(ToolBar | Keys | Save);
		statusBar()->hide();
	}
	else
		setupGUI(ToolBar | Keys | StatusBar | Save | Create);

	loadConfig();

	dockInformation().setWidget(&infoPane());

	infoPane().clear();

	scanDevices();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (applyProgressDialog().isVisible())
	{
		event->ignore();
		return;
	}

	if (numPendingOperations() > 0)
	{
		if (KMessageBox::warningContinueCancel(this,
			i18ncp("@info", "<para>Do you really want to quit the application?</para><para>There is still an operation pending.</para>",
    		"<para>Do you really want to quit the application?</para><para>There are still %1 operations pending.</para>", numPendingOperations()),
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
	if ((event->type() == QEvent::ActivationChange || event->type() == QEvent::WindowStateChange) && event->spontaneous() && isActiveWindow())
	{
		QWidget* w = NULL;

		if (applyProgressDialog().isVisible())
			w = &applyProgressDialog();
		else if (scanProgressDialog().isVisible())
			w = &scanProgressDialog();

		if (w != NULL)
		{
			w->activateWindow();
			w->raise();
			w->setFocus();
		}
	}

	KXmlGuiWindow::changeEvent(event);
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

	// Device actions
	KAction* refreshDevices = actionCollection()->addAction("refreshDevices",  this, SLOT(onRefreshDevices()));
	refreshDevices->setText(i18nc("@action:inmenu refresh list of devices", "Refresh Devices"));
	refreshDevices->setToolTip(i18nc("@info:tooltip", "Refresh all devices"));
	refreshDevices->setStatusTip(i18nc("@info:status", "Renew the devices list."));
	refreshDevices->setShortcut(Qt::Key_F5);
	refreshDevices->setIcon(BarIcon("view-refresh"));

	KAction* createNewPartitionTable = actionCollection()->addAction("createNewPartitionTable", this, SLOT(onCreateNewPartitionTable()));
	createNewPartitionTable->setEnabled(false);
	createNewPartitionTable->setText(i18nc("@action:inmenu", "New Partition Table"));
	createNewPartitionTable->setToolTip(i18nc("@info:tooltip", "Create a new partition table"));
	createNewPartitionTable->setStatusTip(i18nc("@info:status", "Create a new and empty partition table on a device."));
	createNewPartitionTable->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
	createNewPartitionTable->setIcon(BarIcon("edit-clear"));

	KAction* propertiesDevice = actionCollection()->addAction("propertiesDevice", this, SLOT(onPropertiesDevice()));
	propertiesDevice->setText(i18nc("@action:inmenu", "Properties"));
	propertiesDevice->setToolTip(i18nc("@info:tooltip", "Show device properties dialog"));
	propertiesDevice->setStatusTip(i18nc("@info:status", "View and modify device properties"));
	propertiesDevice->setIcon(BarIcon("document-properties"));

	// Partition actions
	KAction* newPartition = actionCollection()->addAction("newPartition", &pmWidget(), SLOT(onNewPartition()));
	newPartition->setEnabled(false);
	newPartition->setText(i18nc("@action:inmenu create a new partition", "New"));
	newPartition->setToolTip(i18nc("@info:tooltip", "New partition"));
	newPartition->setStatusTip(i18nc("@info:status", "Create a new partition."));
	newPartition->setShortcut(Qt::CTRL | Qt::Key_N);
	newPartition->setIcon(BarIcon("document-new"));

	KAction* resizePartition = actionCollection()->addAction("resizePartition", &pmWidget(), SLOT(onResizePartition()));
	resizePartition->setEnabled(false);
	resizePartition->setText(i18nc("@action:inmenu", "Resize/Move"));
	resizePartition->setToolTip(i18nc("@info:tooltip", "Resize or move partition"));
	resizePartition->setStatusTip(i18nc("@info:status", "Shrink, grow or move an existing partition."));
	resizePartition->setShortcut(Qt::CTRL | Qt::Key_R);
	resizePartition->setIcon(BarIcon("arrow-right-double"));

	KAction* deletePartition = actionCollection()->addAction("deletePartition", &pmWidget(), SLOT(onDeletePartition()));
	deletePartition->setEnabled(false);
	deletePartition->setText(i18nc("@action:inmenu", "Delete"));
	deletePartition->setToolTip(i18nc("@info:tooltip", "Delete partition"));
	deletePartition->setStatusTip(i18nc("@info:status", "Delete a partition."));
	deletePartition->setShortcut(Qt::Key_Delete);
	deletePartition->setIcon(BarIcon("edit-delete"));

	KAction* shredPartition = actionCollection()->addAction("shredPartition", &pmWidget(), SLOT(onShredPartition()));
	shredPartition->setEnabled(false);
	shredPartition->setText(i18nc("@action:inmenu", "Shred"));
	shredPartition->setToolTip(i18nc("@info:tooltip", "Shred partition"));
	shredPartition->setStatusTip(i18nc("@info:status", "Shred a partition so that its contents cannot be restored."));
	shredPartition->setShortcut(Qt::SHIFT | Qt::Key_Delete);
	shredPartition->setIcon(BarIcon("edit-delete-shred"));

	KAction* copyPartition = actionCollection()->addAction("copyPartition", &pmWidget(), SLOT(onCopyPartition()));
	copyPartition->setEnabled(false);
	copyPartition->setText(i18nc("@action:inmenu", "Copy"));
	copyPartition->setToolTip(i18nc("@info:tooltip", "Copy partition"));
	copyPartition->setStatusTip(i18nc("@info:status", "Copy an existing partition."));
	copyPartition->setShortcut(Qt::CTRL | Qt::Key_C);
	copyPartition->setIcon(BarIcon("edit-copy"));

	KAction* pastePartition = actionCollection()->addAction("pastePartition", &pmWidget(), SLOT(onPastePartition()));
	pastePartition->setEnabled(false);
	pastePartition->setText(i18nc("@action:inmenu", "Paste"));
	pastePartition->setToolTip(i18nc("@info:tooltip", "Paste partition"));
	pastePartition->setStatusTip(i18nc("@info:status", "Paste a copied partition."));
	pastePartition->setShortcut(Qt::CTRL | Qt::Key_V);
	pastePartition->setIcon(BarIcon("edit-paste"));

	KAction* editMountPoint = actionCollection()->addAction("editMountPoint", &pmWidget(), SLOT(onEditMountPoint()));
	editMountPoint->setEnabled(false);
	editMountPoint->setText(i18nc("@action:inmenu", "Edit Mount Point"));
	editMountPoint->setToolTip(i18nc("@info:tooltip", "Edit mount point"));
	editMountPoint->setStatusTip(i18nc("@info:status", "Edit a partition's mount point and options."));

	KAction* mountPartition = actionCollection()->addAction("mountPartition", &pmWidget(), SLOT(onMountPartition()));
	mountPartition->setEnabled(false);
	mountPartition->setText(i18nc("@action:inmenu", "Mount"));
	mountPartition->setToolTip(i18nc("@info:tooltip", "Mount or unmount partition"));
	mountPartition->setStatusTip(i18nc("@info:status", "Mount or unmount a partition."));

	KAction* checkPartition = actionCollection()->addAction("checkPartition", &pmWidget(), SLOT(onCheckPartition()));
	checkPartition->setEnabled(false);
	checkPartition->setText(i18nc("@action:inmenu", "Check"));
	checkPartition->setToolTip(i18nc("@info:tooltip", "Check partition"));
	checkPartition->setStatusTip(i18nc("@info:status", "Check a filesystem on a partition for errors."));
	checkPartition->setIcon(BarIcon("flag"));

	KAction* propertiesPartition = actionCollection()->addAction("propertiesPartition", &pmWidget(), SLOT(onPropertiesPartition()));
	propertiesPartition->setEnabled(false);
	propertiesPartition->setText(i18nc("@action:inmenu", "Properties"));
	propertiesPartition->setToolTip(i18nc("@info:tooltip", "Show partition properties dialog"));
	propertiesPartition->setStatusTip(i18nc("@info:status", "View and modify partition properties (label, partition flags, etc.)"));
	propertiesPartition->setIcon(BarIcon("document-properties"));

	KAction* backup = actionCollection()->addAction("backupPartition", &pmWidget(), SLOT(onBackupPartition()));
	backup->setEnabled(false);
	backup->setText(i18nc("@action:inmenu", "Backup"));
	backup->setToolTip(i18nc("@info:tooltip", "Backup partition"));
	backup->setStatusTip(i18nc("@info:status", "Backup a partition to an image file."));
	backup->setIcon(BarIcon("document-export"));

	KAction* restore = actionCollection()->addAction("restorePartition", &pmWidget(), SLOT(onRestorePartition()));
	restore->setEnabled(false);
	restore->setText(i18nc("@action:inmenu", "Restore"));
	restore->setToolTip(i18nc("@info:tooltip", "Restore partition"));
	restore->setStatusTip(i18nc("@info:status", "Restore a partition from an image file."));
	restore->setIcon(BarIcon("document-import"));

	// View actions
	KAction* fileSystemSupport = actionCollection()->addAction("fileSystemSupport", this, SLOT(onFileSystemSupport()));
	fileSystemSupport->setText(i18nc("@action:inmenu", "File System Support"));
	fileSystemSupport->setToolTip(i18nc("@info:tooltip", "View file system support information"));
	fileSystemSupport->setStatusTip(i18nc("@info:status", "Show information about supported file systems."));

	actionCollection()->addAction("toggleDockDevices", dockDevices().toggleViewAction());
	actionCollection()->addAction("toggleDockOperations", dockOperations().toggleViewAction());
	actionCollection()->addAction("toggleDockInformation", dockInformation().toggleViewAction());
	actionCollection()->addAction("toggleDockLog", dockLog().toggleViewAction());

	// Settings Actions
	KStandardAction::showMenubar(this, SLOT(onShowMenuBar()), actionCollection());
	KStandardAction::preferences(this, SLOT(onConfigureOptions()), actionCollection());
}

void MainWindow::setupConnections()
{
	connect(&listDevices(), SIGNAL(selectionChanged(const QString&)), &pmWidget(), SLOT(setSelectedDevice(const QString&)));
	connect(&listDevices(), SIGNAL(deviceDoubleClicked(const QString&)), SLOT(onPropertiesDevice(const QString&)));
}

void MainWindow::setupStatusBar()
{
	if (!isKPart())
		statusBar()->addWidget(&statusText());
}

void MainWindow::loadConfig()
{
	if (Config::firstRun())
	{
		dockLog().setVisible(false);
		dockInformation().setVisible(false);
		toolBar("deviceToolBar")->setVisible(false);
	}
}

void MainWindow::saveConfig() const
{
	Config::setFirstRun(false);
	Config::self()->writeConfig();
}

void MainWindow::enableActions()
{
	actionCollection()->action("createNewPartitionTable")->setEnabled(CreatePartitionTableOperation::canCreate(pmWidget().selectedDevice()));

	actionCollection()->action("undoOperation")->setEnabled(numPendingOperations() > 0);
	actionCollection()->action("clearAllOperations")->setEnabled(numPendingOperations() > 0);
	actionCollection()->action("applyAllOperations")->setEnabled(numPendingOperations() > 0 && (geteuid() == 0 || Config::allowApplyOperationsAsNonRoot()));

	const bool readOnly = pmWidget().selectedDevice() == NULL ||
			pmWidget().selectedDevice()->partitionTable() == NULL ||
			pmWidget().selectedDevice()->partitionTable()->isReadOnly();

	const Partition* part = pmWidget().selectedPartition();

	actionCollection()->action("newPartition")->setEnabled(!readOnly && NewOperation::canCreateNew(part));
	const bool canResize = ResizeOperation::canGrow(part) || ResizeOperation::canShrink(part) || ResizeOperation::canMove(part);
	actionCollection()->action("resizePartition")->setEnabled(!readOnly && canResize);
	actionCollection()->action("copyPartition")->setEnabled(CopyOperation::canCopy(part));
	actionCollection()->action("deletePartition")->setEnabled(!readOnly && DeleteOperation::canDelete(part));
	actionCollection()->action("shredPartition")->setEnabled(!readOnly && DeleteOperation::canDelete(part));
	actionCollection()->action("pastePartition")->setEnabled(!readOnly && CopyOperation::canPaste(part, pmWidget().clipboardPartition()));
	actionCollection()->action("propertiesPartition")->setEnabled(part != NULL);

	actionCollection()->action("editMountPoint")->setEnabled(part && part->canMount());
	actionCollection()->action("mountPartition")->setEnabled(part && (part->canMount() || part->canUnmount()));

	if (part != NULL)
		actionCollection()->action("mountPartition")->setText(part->isMounted() ? part->fileSystem().unmountTitle() : part->fileSystem().mountTitle() );

	actionCollection()->action("checkPartition")->setEnabled(!readOnly && CheckOperation::canCheck(part));

	actionCollection()->action("backupPartition")->setEnabled(BackupOperation::canBackup(part));
	actionCollection()->action("restorePartition")->setEnabled(RestoreOperation::canRestore(part));
}

void MainWindow::on_m_ApplyProgressDialog_finished()
{
	scanDevices();
}

void MainWindow::on_m_OperationStack_operationsChanged()
{
	listOperations().updateOperations(operationStack().operations());

	if (!isKPart())
		statusText().setText(i18ncp("@info:status", "One pending operation", "%1 pending operations", numPendingOperations()));
}

void MainWindow::on_m_OperationStack_devicesChanged()
{
	QReadLocker lockDevices(&operationStack().lock());

	listDevices().updateDevices(operationStack().previewDevices());

	if (pmWidget().selectedDevice())
		infoPane().showDevice(dockWidgetArea(&dockInformation()), *pmWidget().selectedDevice());
	else
		infoPane().clear();

	updateWindowTitle();
}

void MainWindow::on_m_DockInformation_dockLocationChanged(Qt::DockWidgetArea)
{
	on_m_PartitionManagerWidget_selectedPartitionChanged(pmWidget().selectedPartition());
}

void MainWindow::updateWindowTitle()
{
	QString title;

	if (pmWidget().selectedDevice())
		title = pmWidget().selectedDevice()->deviceNode() + " - ";

	title += KGlobal::mainComponent().aboutData()->programName() + ' ' + KGlobal::mainComponent().aboutData()->version();

	setWindowTitle(title);
}

void MainWindow::on_m_ListDevices_contextMenuRequested(const QPoint& pos)
{
	KMenu* menu = static_cast<KMenu*>(guiFactory()->container("device", this));

	if (menu)
		menu->exec(pos);
}

void MainWindow::on_m_PartitionManagerWidget_contextMenuRequested(const QPoint& pos)
{
	KMenu* menu = NULL;

	if (pmWidget().selectedPartition() == NULL)
	{
		if (pmWidget().selectedDevice() != NULL)
			menu = static_cast<KMenu*>(guiFactory()->container("device", this));
	}
	else
		menu = static_cast<KMenu*>(guiFactory()->container("partition", this));

	if (menu)
		menu->exec(pos);
}

void MainWindow::on_m_PartitionManagerWidget_deviceDoubleClicked(const Device*)
{
	actionCollection()->action("propertiesDevice")->trigger();
}

void MainWindow::on_m_PartitionManagerWidget_partitionDoubleClicked(const Partition*)
{
	actionCollection()->action("propertiesPartition")->trigger();
}

void MainWindow::on_m_PartitionManagerWidget_selectedPartitionChanged(const Partition* p)
{
	if (p)
		infoPane().showPartition(dockWidgetArea(&dockInformation()), *p);
	else if (pmWidget().selectedDevice())
		infoPane().showDevice(dockWidgetArea(&dockInformation()), *pmWidget().selectedDevice());
	else
		infoPane().clear();

	updateWindowTitle();
	enableActions();
}

bool MainWindow::isKPart() const
{
	// If we were instantiated with an action collection we're supposed to be a KPart
	return m_ActionCollection != NULL;
}

void MainWindow::onShowMenuBar()
{
	QAction* menuBarAction = actionCollection()->action(KStandardAction::name(KStandardAction::ShowMenubar));
	if (menuBarAction->isChecked())
		menuBar()->show();
	else
	{
		const QString accel = menuBarAction->shortcut().toString();
		KMessageBox::information(this, i18nc("@info", "This will hide the menu bar completely. You can show it again by typing %1.", accel), i18nc("@window:title", "Hide Menu Bar"), "hideMenuBarWarning");

		menuBar()->hide();
	}

	Config::self()->setShowMenuBar(menuBarAction->isChecked());
}

void MainWindow::scanDevices()
{
	Log() << i18nc("@info/plain", "Scanning devices...");

	// remember the currently selected device's node
	setSavedSelectedDeviceNode(pmWidget().selectedDevice() ?  pmWidget().selectedDevice()->deviceNode() : QString());

	pmWidget().clear();

	KApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	scanProgressDialog().setEnabled(true);
	scanProgressDialog().show();

	deviceScanner().start();
}

void MainWindow::on_m_DeviceScanner_progressChanged(const QString& device_node, int percent)
{
	scanProgressDialog().setProgress(percent);
	scanProgressDialog().setDeviceName(device_node);
}

void MainWindow::on_m_DeviceScanner_finished()
{
	QReadLocker lockDevices(&operationStack().lock());

	if (!operationStack().previewDevices().isEmpty())
		pmWidget().setSelectedDevice(operationStack().previewDevices()[0]);

	pmWidget().updatePartitions();

	Log() << i18nc("@info/plain", "Scan finished.");
	KApplication::restoreOverrideCursor();

	scanProgressDialog().hide();

	// try to set the seleted device, either from the saved one or just select the
	// first device
	if (!listDevices().setSelectedDevice(savedSelectedDeviceNode()) && !operationStack().previewDevices().isEmpty())
		listDevices().setSelectedDevice(operationStack().previewDevices()[0]->deviceNode());
}

void MainWindow::onRefreshDevices()
{
	if (numPendingOperations() == 0 || KMessageBox::warningContinueCancel(this,
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
		Log() << i18nc("@info/plain", "Applying operations...");

		applyProgressDialog().show();

		operationRunner().setReport(&applyProgressDialog().report());

		// Undo all operations so the runner has a defined starting point
		for (int i = operationStack().operations().size() - 1; i >= 0; i--)
		{
			operationStack().operations()[i]->undo();
			operationStack().operations()[i]->setStatus(Operation::StatusNone);
		}

		pmWidget().updatePartitions();

		operationRunner().start();
	}
}

void MainWindow::onUndoOperation()
{
	Q_ASSERT(numPendingOperations() > 0);

	if (numPendingOperations() == 0)
		return;

	Log() << i18nc("@info/plain", "Undoing operation: %1", operationStack().operations().last()->description());
	operationStack().pop();

	pmWidget().updatePartitions();
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
		Log() << i18nc("@info/plain", "Clearing the list of pending operations.");
		operationStack().clearOperations();

		pmWidget().updatePartitions();
		enableActions();
	}
}

quint32 MainWindow::numPendingOperations() const
{
	return operationStack().size();
}

void MainWindow::onCreateNewPartitionTable()
{
	Q_ASSERT(pmWidget().selectedDevice());

	if (pmWidget().selectedDevice() == NULL)
	{
		kWarning() << "selected device is null.";
		return;
	}

	QPointer<CreatePartitionTableDialog> dlg = new CreatePartitionTableDialog(this, *pmWidget().selectedDevice());

	if (dlg->exec() == KDialog::Accepted)
	{
		operationStack().push(new CreatePartitionTableOperation(*pmWidget().selectedDevice(), dlg->type()));

		pmWidget().updatePartitions();
		enableActions();
		infoPane().showDevice(dockWidgetArea(&dockInformation()), *pmWidget().selectedDevice());
	}

	delete dlg;
}

void MainWindow::onFileSystemSupport()
{
	FileSystemSupportDialog dlg(this);
	dlg.exec();
}

void MainWindow::onSettingsChanged(const QString&)
{
	enableActions();
	pmWidget().updatePartitions();
}

void MainWindow::onConfigureOptions()
{
	if (ConfigureOptionsDialog::showDialog("Settings"))
		return;

	QPointer<ConfigureOptionsDialog> dlg = new ConfigureOptionsDialog(this, "Settings", Config::self());

	connect(dlg, SIGNAL(settingsChanged(const QString&)), SLOT(onSettingsChanged(const QString&)));

	dlg->show();
}

void MainWindow::onPropertiesDevice(const QString&)
{
	Q_ASSERT(pmWidget().selectedDevice());

	if (pmWidget().selectedDevice())
	{
		Device& d = *pmWidget().selectedDevice();

		QPointer<DevicePropsDialog> dlg = new DevicePropsDialog(this, d);
		if (dlg->exec() == KDialog::Accepted)
		{
			if (d.partitionTable()->type() == PartitionTable::msdos && dlg->sectorBasedAlignment())
				d.partitionTable()->setType(d, PartitionTable::msdos_sectorbased);
			else if (d.partitionTable()->type() == PartitionTable::msdos_sectorbased && dlg->cylinderBasedAlignment())
				d.partitionTable()->setType(d, PartitionTable::msdos);

			on_m_OperationStack_devicesChanged();
		}

		delete dlg;
	}
}

