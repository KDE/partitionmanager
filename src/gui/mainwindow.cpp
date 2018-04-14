/*************************************************************************
 *  Copyright (C) 2008, 2009, 2010, 2012 by Volker Lanz <vl@fidra.de>    *
 *  Copyright (C) 2015 by Teo Mrnjavac <teo@kde.org>                     *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "gui/mainwindow.h"
#include "gui/infopane.h"
#include "gui/applyprogressdialog.h"
#include "gui/scanprogressdialog.h"
#include "gui/createpartitiontabledialog.h"
#include "gui/createvolumegroupdialog.h"
#include "gui/resizevolumegroupdialog.h"
#include "gui/filesystemsupportdialog.h"
#include "gui/devicepropsdialog.h"
#include "gui/smartdialog.h"

#include "config/configureoptionsdialog.h"

#include <backend/corebackendmanager.h>
#include <backend/corebackend.h>

#include <core/device.h>
#include <core/partitionalignment.h>
#include <core/smartstatus.h>

#include <ops/operation.h>
#include <ops/createpartitiontableoperation.h>
#include <ops/createvolumegroupoperation.h>
#include <ops/resizevolumegroupoperation.h>
#include <ops/removevolumegroupoperation.h>
#include <ops/deactivatevolumegroupoperation.h>
#include <ops/resizeoperation.h>
#include <ops/copyoperation.h>
#include <ops/deleteoperation.h>
#include <ops/newoperation.h>
#include <ops/backupoperation.h>
#include <ops/restoreoperation.h>
#include <ops/checkoperation.h>

#include <fs/filesystem.h>
#include <fs/filesystemfactory.h>
#include <fs/luks.h>

#include <util/externalcommand.h>
#include <util/helpers.h>
#include <util/guihelpers.h>
#include <util/report.h>

#include <QApplication>
#include <QCloseEvent>
#include <QCollator>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QtGlobal>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QReadLocker>
#include <QRegularExpression>
#include <QStatusBar>
#include <QTemporaryFile>
#include <QTextStream>

#include <KAboutApplicationDialog>
#include <KActionCollection>
#include <KMessageBox>
#include <KAboutData>
#include <KIconLoader>
#include <KLocalizedString>
#include <KXMLGUIFactory>
#include <KJobUiDelegate>
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KJobWidgets>
#include "config.h"

/** Creates a new MainWindow instance.
    @param parent the parent widget
*/
MainWindow::MainWindow(QWidget* parent) :
    KXmlGuiWindow(parent),
    Ui::MainWindowBase(),
    m_OperationStack(new OperationStack(this)),
    m_OperationRunner(new OperationRunner(this, operationStack())),
    m_DeviceScanner(new DeviceScanner(this, operationStack())),
    m_ApplyProgressDialog(new ApplyProgressDialog(this, operationRunner())),
    m_ScanProgressDialog(new ScanProgressDialog(this)),
    m_StatusText(new QLabel(this))
{
    setupObjectNames();
    setupUi(this);
    connect(&m_ListDevices->listDevices(), &QListWidget::customContextMenuRequested, this, &MainWindow::listDevicesContextMenuRequested);
    connect(&m_TreeLog->treeLog(), &QTreeWidget::customContextMenuRequested, this, &MainWindow::treeLogContextMenuRequested);
    connect(&m_ListOperations->listOperations(), &QListWidget::customContextMenuRequested, this, &MainWindow::listOperationsContextMenuRequested);
    init();
}

void MainWindow::setupObjectNames()
{
    m_OperationStack->setObjectName(QStringLiteral("m_OperationStack"));
    m_OperationRunner->setObjectName(QStringLiteral("m_OperationRunner"));
    m_DeviceScanner->setObjectName(QStringLiteral("m_DeviceScanner"));
    m_ApplyProgressDialog->setObjectName(QStringLiteral("m_ApplyProgressDialog"));
    m_ScanProgressDialog->setObjectName(QStringLiteral("m_ScanProgressDialog"));
}

void MainWindow::init()
{
    treeLog().init();

    connect(GlobalLog::instance(), &GlobalLog::newMessage, &treeLog(), &TreeLog::onNewLogMessage);

    setupActions();
    setupStatusBar();
    setupConnections();

    listDevices().setActionCollection(actionCollection());
    listOperations().setActionCollection(actionCollection());
    pmWidget().init(&operationStack());

    setupGUI();

    loadConfig();

    scanDevices();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (applyProgressDialog().isVisible()) {
        event->ignore();
        return;
    }

    if (operationStack().size() > 0) {
        if (KMessageBox::warningContinueCancel(this,
                                               xi18ncp("@info", "<para>Do you really want to quit the application?</para><para>There is still an operation pending.</para>",
                                                       "<para>Do you really want to quit the application?</para><para>There are still %1 operations pending.</para>", operationStack().size()),
                                               xi18nc("@title:window", "Discard Pending Operations and Quit?"),
                                               KGuiItem(xi18nc("@action:button", "Quit <application>%1</application>", QGuiApplication::applicationDisplayName()), QStringLiteral("arrow-right")),
                                               KStandardGuiItem::cancel(), QStringLiteral("reallyQuit")) == KMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    saveConfig();

    KXmlGuiWindow::closeEvent(event);
    ExternalCommand::stopHelper();
}

void MainWindow::changeEvent(QEvent* event)
{
    if ((event->type() == QEvent::ActivationChange || event->type() == QEvent::WindowStateChange) && event->spontaneous() && isActiveWindow()) {
        QWidget* w = nullptr;

        if (applyProgressDialog().isVisible())
            w = &applyProgressDialog();
        else if (scanProgressDialog().isVisible())
            w = &scanProgressDialog();

        if (w != nullptr) {
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
    KStandardAction::quit(this, &MainWindow::close, actionCollection());

    // Edit actions
    QAction* undoOperation = actionCollection()->addAction(QStringLiteral("undoOperation"));
    connect(undoOperation, &QAction::triggered, this, &MainWindow::onUndoOperation);
    undoOperation->setEnabled(false);
    undoOperation->setText(xi18nc("@action:inmenu", "Undo"));
    undoOperation->setToolTip(xi18nc("@info:tooltip", "Undo the last operation"));
    undoOperation->setStatusTip(xi18nc("@info:status", "Remove the last operation from the list."));
    actionCollection()->setDefaultShortcut(undoOperation, QKeySequence(Qt::CTRL + Qt::Key_Z));
    undoOperation->setIcon(QIcon::fromTheme(QStringLiteral("edit-undo")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* clearAllOperations = actionCollection()->addAction(QStringLiteral("clearAllOperations"));
    connect(clearAllOperations, &QAction::triggered, this, &MainWindow::onClearAllOperations);
    clearAllOperations->setEnabled(false);
    clearAllOperations->setText(xi18nc("@action:inmenu clear the list of operations", "Clear"));
    clearAllOperations->setToolTip(xi18nc("@info:tooltip", "Clear all operations"));
    clearAllOperations->setStatusTip(xi18nc("@info:status", "Empty the list of pending operations."));
    clearAllOperations->setIcon(QIcon::fromTheme(QStringLiteral("dialog-cancel")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* applyAllOperations = actionCollection()->addAction(QStringLiteral("applyAllOperations"));
    connect(applyAllOperations, &QAction::triggered, this, &MainWindow::onApplyAllOperations);
    applyAllOperations->setEnabled(false);
    applyAllOperations->setText(xi18nc("@action:inmenu apply all operations", "Apply"));
    applyAllOperations->setToolTip(xi18nc("@info:tooltip", "Apply all operations"));
    applyAllOperations->setStatusTip(xi18nc("@info:status", "Apply the pending operations in the list."));
    applyAllOperations->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")).pixmap(IconSize(KIconLoader::Toolbar)));

    // Device actions
    QAction* createNewPartitionTable = actionCollection()->addAction(QStringLiteral("createNewPartitionTable"));
    connect(createNewPartitionTable, &QAction::triggered, this, &MainWindow::onCreateNewPartitionTable);
    createNewPartitionTable->setEnabled(false);
    createNewPartitionTable->setText(xi18nc("@action:inmenu", "New Partition Table"));
    createNewPartitionTable->setToolTip(xi18nc("@info:tooltip", "Create a new partition table"));
    createNewPartitionTable->setStatusTip(xi18nc("@info:status", "Create a new and empty partition table on a device."));
    actionCollection()->setDefaultShortcut(createNewPartitionTable, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    createNewPartitionTable->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* exportPartitionTable = actionCollection()->addAction(QStringLiteral("exportPartitionTable"));
    connect(exportPartitionTable, &QAction::triggered, this, &MainWindow::onExportPartitionTable);
    exportPartitionTable->setEnabled(false);
    exportPartitionTable->setText(xi18nc("@action:inmenu", "Export Partition Table"));
    exportPartitionTable->setToolTip(xi18nc("@info:tooltip", "Export a partition table"));
    exportPartitionTable->setStatusTip(xi18nc("@info:status", "Export the device's partition table to a text file."));
    exportPartitionTable->setIcon(QIcon::fromTheme(QStringLiteral("document-export")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* importPartitionTable = actionCollection()->addAction(QStringLiteral("importPartitionTable"));
    connect(importPartitionTable, &QAction::triggered, this, &MainWindow::onImportPartitionTable);
    importPartitionTable->setEnabled(false);
    importPartitionTable->setText(xi18nc("@action:inmenu", "Import Partition Table"));
    importPartitionTable->setToolTip(xi18nc("@info:tooltip", "Import a partition table"));
    importPartitionTable->setStatusTip(xi18nc("@info:status", "Import a partition table from a text file."));
    importPartitionTable->setIcon(QIcon::fromTheme(QStringLiteral("document-import")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* removeVolumeGroup = actionCollection()->addAction(QStringLiteral("removeVolumeGroup"));
    connect(removeVolumeGroup, &QAction::triggered, this, &MainWindow::onRemoveVolumeGroup);
    removeVolumeGroup->setEnabled(false);
    removeVolumeGroup->setVisible(false);
    removeVolumeGroup->setText(i18nc("@action:inmenu", "Remove Volume Group"));
    removeVolumeGroup->setToolTip(i18nc("@info:tooltip", "Remove selected Volume Group"));
    removeVolumeGroup->setStatusTip(i18nc("@info:status", "Remove selected Volume Group."));
    //actionCollection()->setDefaultShortcut(removeVolumeGroup, QKeySequence(/*SHORTCUT KEY HERE*/));
    removeVolumeGroup->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* resizeVolumeGroup = actionCollection()->addAction(QStringLiteral("resizeVolumeGroup"));
    connect(resizeVolumeGroup, &QAction::triggered, this, &MainWindow::onResizeVolumeGroup);
    resizeVolumeGroup->setEnabled(false);
    resizeVolumeGroup->setVisible(false);
    resizeVolumeGroup->setText(i18nc("@action:inmenu", "Resize Volume Group"));
    resizeVolumeGroup->setToolTip(i18nc("@info:tooltip", "Resize selected Volume Group"));
    resizeVolumeGroup->setStatusTip(i18nc("@info:status", "Extend or reduce selected Volume Group."));
    //actionCollection()->setDefaultShortcut(resizeVolumeGroup, QKeySequence(/*SHORTCUT KEY HERE*/));
    resizeVolumeGroup->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right-double")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* deactivateVolumeGroup = actionCollection()->addAction(QStringLiteral("deactivateVolumeGroup"));
    connect(deactivateVolumeGroup, &QAction::triggered, this, &MainWindow::onDeactivateVolumeGroup);
    deactivateVolumeGroup->setEnabled(false);
    deactivateVolumeGroup->setVisible(false);
    deactivateVolumeGroup->setText(i18nc("@action:inmenu", "Deactivate Volume Group"));
    deactivateVolumeGroup->setToolTip(i18nc("@info:tooltip", "Deactivate selected Volume Group"));
    deactivateVolumeGroup->setStatusTip(i18nc("@info:status", "Deactivate selected Volume Group before unplugging removable devices."));
    deactivateVolumeGroup->setIcon(QIcon::fromTheme(QStringLiteral("media-eject")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* smartStatusDevice = actionCollection()->addAction(QStringLiteral("smartStatusDevice"));
    connect(smartStatusDevice, &QAction::triggered, this, &MainWindow::onSmartStatusDevice);
    smartStatusDevice->setEnabled(false);
    smartStatusDevice->setText(xi18nc("@action:inmenu", "SMART Status"));
    smartStatusDevice->setToolTip(xi18nc("@info:tooltip", "Show SMART status"));
    smartStatusDevice->setStatusTip(xi18nc("@info:status", "Show the device's SMART status if supported"));

    QAction* propertiesDevice = actionCollection()->addAction(QStringLiteral("propertiesDevice"));
    connect(propertiesDevice, &QAction::triggered, [this] {onPropertiesDevice({});});
    propertiesDevice->setEnabled(false);
    propertiesDevice->setText(xi18nc("@action:inmenu", "Properties"));
    propertiesDevice->setToolTip(xi18nc("@info:tooltip", "Show device properties dialog"));
    propertiesDevice->setStatusTip(xi18nc("@info:status", "View and modify device properties"));
    propertiesDevice->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")).pixmap(IconSize(KIconLoader::Toolbar)));

    // Partition actions
    QAction* newPartition = actionCollection()->addAction(QStringLiteral("newPartition"));
    connect(newPartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onNewPartition);
    newPartition->setEnabled(false);
    newPartition->setText(xi18nc("@action:inmenu create a new partition", "New"));
    newPartition->setToolTip(xi18nc("@info:tooltip", "New partition"));
    newPartition->setStatusTip(xi18nc("@info:status", "Create a new partition."));
    actionCollection()->setDefaultShortcut(newPartition, QKeySequence(Qt::CTRL + Qt::Key_N));
    newPartition->setIcon(QIcon::fromTheme(QStringLiteral("document-new")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* resizePartition = actionCollection()->addAction(QStringLiteral("resizePartition"));
    connect(resizePartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onResizePartition);
    resizePartition->setEnabled(false);
    resizePartition->setText(xi18nc("@action:inmenu", "Resize/Move"));
    resizePartition->setToolTip(xi18nc("@info:tooltip", "Resize or move partition"));
    resizePartition->setStatusTip(xi18nc("@info:status", "Shrink, grow or move an existing partition."));
    actionCollection()->setDefaultShortcut(resizePartition, QKeySequence(Qt::CTRL + Qt::Key_R));
    resizePartition->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right-double")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* deletePartition = actionCollection()->addAction(QStringLiteral("deletePartition"));
    connect(deletePartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onDeletePartition);
    deletePartition->setEnabled(false);
    deletePartition->setText(xi18nc("@action:inmenu", "Delete"));
    deletePartition->setToolTip(xi18nc("@info:tooltip", "Delete partition"));
    deletePartition->setStatusTip(xi18nc("@info:status", "Delete a partition."));
    actionCollection()->setDefaultShortcut(deletePartition, QKeySequence::Delete);
    deletePartition->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* shredPartition = actionCollection()->addAction(QStringLiteral("shredPartition"));
    connect(shredPartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onShredPartition);
    shredPartition->setEnabled(false);
    shredPartition->setText(xi18nc("@action:inmenu", "Shred"));
    shredPartition->setToolTip(xi18nc("@info:tooltip", "Shred partition"));
    shredPartition->setStatusTip(xi18nc("@info:status", "Shred a partition so that its contents cannot be restored."));
    actionCollection()->setDefaultShortcut(shredPartition, QKeySequence(Qt::SHIFT + Qt::Key_Delete));
    shredPartition->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete-shred")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* copyPartition = actionCollection()->addAction(QStringLiteral("copyPartition"));
    connect(copyPartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onCopyPartition);
    copyPartition->setEnabled(false);
    copyPartition->setText(xi18nc("@action:inmenu", "Copy"));
    copyPartition->setToolTip(xi18nc("@info:tooltip", "Copy partition"));
    copyPartition->setStatusTip(xi18nc("@info:status", "Copy an existing partition."));
    actionCollection()->setDefaultShortcut(copyPartition, QKeySequence(Qt::CTRL + Qt::Key_C));
    copyPartition->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* pastePartition = actionCollection()->addAction(QStringLiteral("pastePartition"));
    connect(pastePartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onPastePartition);
    pastePartition->setEnabled(false);
    pastePartition->setText(xi18nc("@action:inmenu", "Paste"));
    pastePartition->setToolTip(xi18nc("@info:tooltip", "Paste partition"));
    pastePartition->setStatusTip(xi18nc("@info:status", "Paste a copied partition."));
    actionCollection()->setDefaultShortcut(pastePartition, QKeySequence(Qt::CTRL + Qt::Key_V));
    pastePartition->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* editMountPoint = actionCollection()->addAction(QStringLiteral("editMountPoint"));
    connect(editMountPoint, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onEditMountPoint);
    editMountPoint->setEnabled(false);
    editMountPoint->setText(xi18nc("@action:inmenu", "Edit Mount Point"));
    editMountPoint->setToolTip(xi18nc("@info:tooltip", "Edit mount point"));
    editMountPoint->setStatusTip(xi18nc("@info:status", "Edit a partition's mount point and options."));

    QAction* mountPartition = actionCollection()->addAction(QStringLiteral("mountPartition"));
    connect(mountPartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onMountPartition);
    mountPartition->setEnabled(false);
    mountPartition->setText(xi18nc("@action:inmenu", "Mount"));
    mountPartition->setToolTip(xi18nc("@info:tooltip", "Mount or unmount partition"));
    mountPartition->setStatusTip(xi18nc("@info:status", "Mount or unmount a partition."));

    QAction* decryptPartition = actionCollection()->addAction(QStringLiteral("decryptPartition"));
    connect(decryptPartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onDecryptPartition);
    decryptPartition->setEnabled(false);
    decryptPartition->setText(xi18nc("@action:inmenu", "Unlock"));
    decryptPartition->setToolTip(xi18nc("@info:tooltip", "Unlock or lock encrypted partition"));
    decryptPartition->setStatusTip(xi18nc("@info:status", "Unlock or lock encrypted partition."));

    QAction* checkPartition = actionCollection()->addAction(QStringLiteral("checkPartition"));
    connect(checkPartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onCheckPartition);
    checkPartition->setEnabled(false);
    checkPartition->setText(xi18nc("@action:inmenu", "Check"));
    checkPartition->setToolTip(xi18nc("@info:tooltip", "Check partition"));
    checkPartition->setStatusTip(xi18nc("@info:status", "Check a filesystem on a partition for errors."));
    checkPartition->setIcon(QIcon::fromTheme(QStringLiteral("flag")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* propertiesPartition = actionCollection()->addAction(QStringLiteral("propertiesPartition"));
    connect(propertiesPartition, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onPropertiesPartition);
    propertiesPartition->setEnabled(false);
    propertiesPartition->setText(xi18nc("@action:inmenu", "Properties"));
    propertiesPartition->setToolTip(xi18nc("@info:tooltip", "Show partition properties dialog"));
    propertiesPartition->setStatusTip(xi18nc("@info:status", "View and modify partition properties (label, partition flags, etc.)"));
    propertiesPartition->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* backup = actionCollection()->addAction(QStringLiteral("backupPartition"));
    connect(backup, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onBackupPartition);
    backup->setEnabled(false);
    backup->setText(xi18nc("@action:inmenu", "Backup"));
    backup->setToolTip(xi18nc("@info:tooltip", "Backup partition"));
    backup->setStatusTip(xi18nc("@info:status", "Backup a partition to an image file."));
    backup->setIcon(QIcon::fromTheme(QStringLiteral("document-export")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* restore = actionCollection()->addAction(QStringLiteral("restorePartition"));
    connect(restore, &QAction::triggered, &pmWidget(), &PartitionManagerWidget::onRestorePartition);
    restore->setEnabled(false);
    restore->setText(xi18nc("@action:inmenu", "Restore"));
    restore->setToolTip(xi18nc("@info:tooltip", "Restore partition"));
    restore->setStatusTip(xi18nc("@info:status", "Restore a partition from an image file."));
    restore->setIcon(QIcon::fromTheme(QStringLiteral("document-import")).pixmap(IconSize(KIconLoader::Toolbar)));

    // Tools actions
    QAction* createVolumeGroup = actionCollection()->addAction(QStringLiteral("createVolumeGroup"));
    connect(createVolumeGroup, &QAction::triggered, this, &MainWindow::onCreateNewVolumeGroup);
    createVolumeGroup->setEnabled(false);
    createVolumeGroup->setText(i18nc("@action:inmenu", "New Volume Group"));
    createVolumeGroup->setToolTip(i18nc("@info:tooltip", "Create a new LVM Volume Group"));
    createVolumeGroup->setStatusTip(i18nc("@info:status", "Create a new LVM Volume Group as a device."));
    actionCollection()->setDefaultShortcut(createVolumeGroup, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
    createVolumeGroup->setIcon(QIcon::fromTheme(QStringLiteral("document-new")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* fileSystemSupport = actionCollection()->addAction(QStringLiteral("fileSystemSupport"));
    connect(fileSystemSupport, &QAction::triggered, this, &MainWindow::onFileSystemSupport);
    fileSystemSupport->setText(xi18nc("@action:inmenu", "File System Support"));
    fileSystemSupport->setToolTip(xi18nc("@info:tooltip", "View file system support information"));
    fileSystemSupport->setStatusTip(xi18nc("@info:status", "Show information about supported file systems."));

    QAction* refreshDevices = actionCollection()->addAction(QStringLiteral("refreshDevices"));
    connect(refreshDevices, &QAction::triggered, this, &MainWindow::onRefreshDevices);
    refreshDevices->setText(xi18nc("@action:inmenu refresh list of devices", "Refresh Devices"));
    refreshDevices->setToolTip(xi18nc("@info:tooltip", "Refresh all devices"));
    refreshDevices->setStatusTip(xi18nc("@info:status", "Renew the devices list."));
    actionCollection()->setDefaultShortcut(refreshDevices, Qt::Key_F5);
    refreshDevices->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")).pixmap(IconSize(KIconLoader::Toolbar)));

    // Settings Actions
    actionCollection()->addAction(QStringLiteral("toggleDockDevices"), dockDevices().toggleViewAction());
    actionCollection()->addAction(QStringLiteral("toggleDockOperations"), dockOperations().toggleViewAction());
    actionCollection()->addAction(QStringLiteral("toggleDockInformation"), dockInformation().toggleViewAction());
    actionCollection()->addAction(QStringLiteral("toggleDockLog"), dockLog().toggleViewAction());

    KStandardAction::preferences(this, &MainWindow::onConfigureOptions, actionCollection());

    // Log Actions
    QAction* clearLog = actionCollection()->addAction(QStringLiteral("clearLog"));
    connect(clearLog, &QAction::triggered, &treeLog(), &TreeLog::onClearLog);
    clearLog->setText(xi18nc("@action:inmenu", "Clear Log"));
    clearLog->setToolTip(xi18nc("@info:tooltip", "Clear the log output"));
    clearLog->setStatusTip(xi18nc("@info:status", "Clear the log output panel."));
    clearLog->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-list")).pixmap(IconSize(KIconLoader::Toolbar)));

    QAction* saveLog = actionCollection()->addAction(QStringLiteral("saveLog"));
    connect(saveLog, &QAction::triggered, &treeLog(), &TreeLog::onSaveLog);
    saveLog->setText(xi18nc("@action:inmenu", "Save Log"));
    saveLog->setToolTip(xi18nc("@info:tooltip", "Save the log output"));
    saveLog->setStatusTip(xi18nc("@info:status", "Save the log output to a file."));
    saveLog->setIcon(QIcon::fromTheme(QStringLiteral("document-save")).pixmap(IconSize(KIconLoader::Toolbar)));

    // Help Actions
    QAction* aboutKPMcore = actionCollection()->addAction(QStringLiteral("aboutKPMcore"));
    connect(aboutKPMcore, &QAction::triggered, this, &MainWindow::onShowAboutKPMcore);
    aboutKPMcore->setText(xi18nc("@action:inmenu", "About KPMcore Library"));
    aboutKPMcore->setToolTip(xi18nc("@info:tooltip", "Show About KPMcore dialog"));
    aboutKPMcore->setStatusTip(xi18nc("@info:status", "Show About KPMcore dialog."));
    aboutKPMcore->setIcon(QIcon::fromTheme(QStringLiteral("partitionmanager")).pixmap(IconSize(KIconLoader::Toolbar)));
}

void MainWindow::setupConnections()
{
    connect(&listDevices(), &ListDevices::selectionChanged, &pmWidget(), qOverload<const QString&>(&PartitionManagerWidget::setSelectedDevice));
    connect(&listDevices(), &ListDevices::deviceDoubleClicked, this, &MainWindow::onPropertiesDevice);
}

void MainWindow::setupStatusBar()
{
    statusBar()->addWidget(&statusText());
}

void MainWindow::loadConfig()
{
    if (Config::firstRun()) {
        dockLog().setVisible(false);
        dockInformation().setVisible(false);
    }
    PartitionAlignment::setSectorAlignment(Config::sectorAlignment());
}

void MainWindow::saveConfig() const
{
    Config::setFirstRun(false);
    Config::self()->save();
}

void MainWindow::enableActions()
{
    actionCollection()->action(QStringLiteral("createNewPartitionTable"))
            ->setEnabled(CreatePartitionTableOperation::canCreate(pmWidget().selectedDevice()));
    actionCollection()->action(QStringLiteral("createNewPartitionTable"))
            ->setVisible(pmWidget().selectedDevice() && pmWidget().selectedDevice()->type() == Device::Type::Disk_Device);
    actionCollection()->action(QStringLiteral("exportPartitionTable"))
            ->setEnabled(pmWidget().selectedDevice() &&
                         pmWidget().selectedDevice()->partitionTable() &&
                         operationStack().size() == 0);
    actionCollection()->action(QStringLiteral("importPartitionTable"))
            ->setEnabled(CreatePartitionTableOperation::canCreate(pmWidget().selectedDevice()));
    actionCollection()->action(QStringLiteral("smartStatusDevice"))
            ->setEnabled(pmWidget().selectedDevice() != nullptr && pmWidget().selectedDevice()->type() == Device::Type::Disk_Device &&
                                                        pmWidget().selectedDevice()->smartStatus().isValid());
    actionCollection()->action(QStringLiteral("smartStatusDevice"))
            ->setVisible(pmWidget().selectedDevice() != nullptr && pmWidget().selectedDevice()->type() == Device::Type::Disk_Device);
    actionCollection()->action(QStringLiteral("propertiesDevice"))
            ->setEnabled(pmWidget().selectedDevice() != nullptr);

    actionCollection()->action(QStringLiteral("undoOperation"))
            ->setEnabled(operationStack().size() > 0);
    actionCollection()->action(QStringLiteral("clearAllOperations"))
            ->setEnabled(operationStack().size() > 0);
    actionCollection()->action(QStringLiteral("applyAllOperations"))
            ->setEnabled(operationStack().size() > 0);

    const bool readOnly = pmWidget().selectedDevice() == nullptr ||
                          pmWidget().selectedDevice()->partitionTable() == nullptr ||
                          pmWidget().selectedDevice()->partitionTable()->isReadOnly();

    actionCollection()->action(QStringLiteral("createVolumeGroup"))
            ->setEnabled(CreateVolumeGroupOperation::canCreate());

    bool lvmDevice = pmWidget().selectedDevice() && pmWidget().selectedDevice()->type() == Device::Type::LVM_Device;
    bool removable = false;

    if (lvmDevice)
        removable = RemoveVolumeGroupOperation::isRemovable(dynamic_cast<LvmDevice*>(pmWidget().selectedDevice()));

    actionCollection()->action(QStringLiteral("removeVolumeGroup"))->setEnabled(removable);
    actionCollection()->action(QStringLiteral("removeVolumeGroup"))->setVisible(lvmDevice);

    bool deactivatable = lvmDevice ?
        DeactivateVolumeGroupOperation::isDeactivatable(dynamic_cast<LvmDevice*>(pmWidget().selectedDevice())) : false;
    actionCollection()->action(QStringLiteral("deactivateVolumeGroup"))->setEnabled(deactivatable);
    actionCollection()->action(QStringLiteral("deactivateVolumeGroup"))->setVisible(lvmDevice);

    actionCollection()->action(QStringLiteral("resizeVolumeGroup"))->setEnabled(lvmDevice);
    actionCollection()->action(QStringLiteral("resizeVolumeGroup"))->setVisible(lvmDevice);

    const Partition* part = pmWidget().selectedPartition();

    actionCollection()->action(QStringLiteral("newPartition"))
            ->setEnabled(!readOnly && NewOperation::canCreateNew(part));

    const bool canResize = ResizeOperation::canGrow(part) ||
                           ResizeOperation::canShrink(part) ||
                           ResizeOperation::canMove(part);
    actionCollection()->action(QStringLiteral("resizePartition"))
            ->setEnabled(!readOnly && canResize);

    actionCollection()->action(QStringLiteral("copyPartition"))
            ->setEnabled(CopyOperation::canCopy(part));
    actionCollection()->action(QStringLiteral("deletePartition"))
            ->setEnabled(!readOnly && DeleteOperation::canDelete(part));
    actionCollection()->action(QStringLiteral("shredPartition"))
            ->setEnabled(!readOnly && DeleteOperation::canDelete(part));
    actionCollection()->action(QStringLiteral("pastePartition"))
            ->setEnabled(!readOnly && CopyOperation::canPaste(part, pmWidget().clipboardPartition()));
    actionCollection()->action(QStringLiteral("propertiesPartition"))
            ->setEnabled(part != nullptr);

    actionCollection()->action(QStringLiteral("editMountPoint"))
            ->setEnabled(part && !part->isMounted() && part->fileSystem().canMount(part->deviceNode(), QStringLiteral("/")));

    actionCollection()->action(QStringLiteral("mountPartition"))
            ->setEnabled(part &&
                         (part->canMount() || part->canUnmount()));
    if (part != nullptr)
        actionCollection()->action(QStringLiteral("mountPartition"))
                ->setText(part->isMounted() ?
                          part->fileSystem().unmountTitle() :
                          part->fileSystem().mountTitle());

    if (part && part->roles().has(PartitionRole::Luks)) {
        const FileSystem& fsRef = part->fileSystem();
        const FS::luks* luksFs = dynamic_cast<const FS::luks*>(&fsRef);

        actionCollection()->action(QStringLiteral("decryptPartition"))
                ->setVisible(true);
        actionCollection()->action(QStringLiteral("decryptPartition"))
                ->setEnabled(luksFs && !operationStack().contains(part) &&
                             (luksFs->canCryptOpen(part->partitionPath()) ||
                              luksFs->canCryptClose(part->partitionPath())));
        if (luksFs) {
            actionCollection()->action(QStringLiteral("decryptPartition"))
                    ->setText(luksFs->isCryptOpen() ?
                              luksFs->cryptCloseTitle() :
                              luksFs->cryptOpenTitle());
        }
    }
    else {
        actionCollection()->action(QStringLiteral("decryptPartition"))
                ->setEnabled(false);
        actionCollection()->action(QStringLiteral("decryptPartition"))
                ->setVisible(false);
    }


    actionCollection()->action(QStringLiteral("checkPartition"))
            ->setEnabled(!readOnly && CheckOperation::canCheck(part));

    actionCollection()->action(QStringLiteral("backupPartition"))
            ->setEnabled(BackupOperation::canBackup(part));
    actionCollection()->action(QStringLiteral("restorePartition"))
            ->setEnabled(RestoreOperation::canRestore(part));
}

void MainWindow::on_m_ApplyProgressDialog_finished()
{
    scanDevices();
}

void MainWindow::on_m_OperationStack_operationsChanged()
{
    listOperations().updateOperations(operationStack().operations());
    pmWidget().updatePartitions();
    enableActions();

    // this will make sure that the info pane gets updated
    on_m_PartitionManagerWidget_selectedPartitionChanged(pmWidget().selectedPartition());

    statusText().setText(xi18ncp("@info:status", "One pending operation", "%1 pending operations", operationStack().size()));
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
        title = pmWidget().selectedDevice()->deviceNode();

    setWindowTitle(title);
}

void MainWindow::listOperationsContextMenuRequested(const QPoint& pos)
{
    QMenu* menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("edit"), this));

    if (menu)
        menu->exec(m_ListOperations->listOperations().viewport()->mapToGlobal(pos));
}

void MainWindow::treeLogContextMenuRequested(const QPoint& pos)
{
    QMenu* menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("log"), this));

    if (menu)
        menu->exec(m_TreeLog->treeLog().viewport()->mapToGlobal(pos));
}

void MainWindow::listDevicesContextMenuRequested(const QPoint& pos)
{
    QMenu* menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("device"), this));

    if (menu)
        menu->exec(m_ListDevices->listDevices().viewport()->mapToGlobal(pos));
}

void MainWindow::on_m_PartitionManagerWidget_contextMenuRequested(const QPoint& pos)
{
    QMenu* menu = nullptr;

    if (pmWidget().selectedPartition() == nullptr) {
        if (pmWidget().selectedDevice() != nullptr)
            menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("device"), this));
    } else
        menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("partition"), this));

    if (menu)
        menu->exec(pos);
}

void MainWindow::on_m_PartitionManagerWidget_deviceDoubleClicked(const Device*)
{
    actionCollection()->action(QStringLiteral("propertiesDevice"))->trigger();
}

void MainWindow::on_m_PartitionManagerWidget_partitionDoubleClicked(const Partition*)
{
    actionCollection()->action(QStringLiteral("propertiesPartition"))->trigger();
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

void MainWindow::scanDevices()
{
    Log(Log::Level::information) << xi18nc("@info:progress", "Using backend plugin: %1 (%2)",
                                   CoreBackendManager::self()->backend()->id(),
                                   CoreBackendManager::self()->backend()->version());

    Log() << xi18nc("@info:progress", "Scanning devices...");

    // remember the currently selected device's node
    setSavedSelectedDeviceNode(pmWidget().selectedDevice() ?  pmWidget().selectedDevice()->deviceNode() : QString());

    pmWidget().clear();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    scanProgressDialog().setEnabled(true);
    scanProgressDialog().show();

    deviceScanner().start();
}

void MainWindow::on_m_DeviceScanner_progress(const QString& deviceNode, int percent)
{
    scanProgressDialog().setProgress(percent);
    scanProgressDialog().setDeviceName(deviceNode);
}

void MainWindow::on_m_DeviceScanner_finished()
{
    QReadLocker lockDevices(&operationStack().lock());

    scanProgressDialog().setProgress(100);

    if (!operationStack().previewDevices().isEmpty())
        pmWidget().setSelectedDevice(operationStack().previewDevices()[0]);

    pmWidget().updatePartitions();

    Log() << xi18nc("@info:progress", "Scan finished.");
    QApplication::restoreOverrideCursor();

    // try to set the seleted device, either from the saved one or just select the
    // first device
    if (!listDevices().setSelectedDevice(savedSelectedDeviceNode()) && !operationStack().previewDevices().isEmpty())
        listDevices().setSelectedDevice(operationStack().previewDevices()[0]->deviceNode());

    updateSeletedDeviceMenu();
    checkFileSystemSupport();
}

void MainWindow::updateSeletedDeviceMenu()
{
    QMenu* devicesMenu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("selectedDevice"), this));
    devicesMenu->clear();

    devicesMenu->setEnabled(!operationStack().previewDevices().isEmpty());

    const auto previewDevices = operationStack().previewDevices();
    for (auto const &d : previewDevices) {
        QAction* action = new QAction(d->prettyName(), devicesMenu);
        action->setCheckable(true);
        action->setChecked(d->deviceNode() == pmWidget().selectedDevice()->deviceNode());
        action->setData(d->deviceNode());
        connect(action, &QAction::triggered, this, &MainWindow::onSelectedDeviceMenuTriggered);
        devicesMenu->addAction(action);
    }
}

void MainWindow::onSelectedDeviceMenuTriggered(bool)
{
    QAction* action = qobject_cast<QAction*>(sender());
    QMenu* devicesMenu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("selectedDevice"), this));

    if (action == nullptr || action->parent() != devicesMenu)
        return;

    const auto children = devicesMenu->findChildren<QAction*>();
    for (auto &entry : children)
        entry->setChecked(entry == action);

    listDevices().setSelectedDevice(action->data().toString());
}

void MainWindow::on_m_ListDevices_selectionChanged(const QString& device_node)
{
    QMenu* devicesMenu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("selectedDevice"), this));

    const auto children = devicesMenu->findChildren<QAction*>();
    for (auto &entry : children)
        entry->setChecked(entry->data().toString() == device_node);
}

void MainWindow::onRefreshDevices()
{
    if (operationStack().size() == 0 || KMessageBox::warningContinueCancel(this,
            xi18nc("@info",
                   "<para>Do you really want to rescan the devices?</para>"
                   "<para><warning>This will also clear the list of pending operations.</warning></para>"),
            xi18nc("@title:window", "Really Rescan the Devices?"),
            KGuiItem(xi18nc("@action:button", "Rescan Devices"), QStringLiteral("arrow-right")),
            KStandardGuiItem::cancel(), QStringLiteral("reallyRescanDevices")) == KMessageBox::Continue) {
        scanDevices();
    }
}

void MainWindow::onApplyAllOperations()
{
    QStringList opList;

    const auto operations = operationStack().operations();
    for (const auto &op : operations)
        opList.append(op->description());

    if (KMessageBox::warningContinueCancelList(this,
            xi18nc("@info",
                   "<para>Do you really want to apply the pending operations listed below?</para>"
                   "<para><warning>This will permanently modify your disks.</warning></para>"),
            opList, xi18nc("@title:window", "Apply Pending Operations?"),
            KGuiItem(xi18nc("@action:button", "Apply Pending Operations"), QStringLiteral("arrow-right")),
            KStandardGuiItem::cancel()) == KMessageBox::Continue) {
        Log() << xi18nc("@info:status", "Applying operations...");

        applyProgressDialog().show();

        operationRunner().setReport(&applyProgressDialog().report());

        // Undo all operations so the runner has a defined starting point
        for (int i = operationStack().operations().size() - 1; i >= 0; i--) {
            operationStack().operations()[i]->undo();
            operationStack().operations()[i]->setStatus(Operation::StatusNone);
        }

        pmWidget().updatePartitions();

        operationRunner().start();
    }
}

void MainWindow::onUndoOperation()
{
    Q_ASSERT(operationStack().size() > 0);

    if (operationStack().size() == 0)
        return;

    Log() << xi18nc("@info:status", "Undoing operation: %1", operationStack().operations().last()->description());
    operationStack().pop();

    // it's possible the undo killed the partition in the clipboard. if there's a partition in the clipboard, try
    // to find a device for it (OperationStack::findDeviceForPartition() only compares pointers, so an invalid
    // pointer is not a problem). if no device is found, the pointer must be dangling, so clear the clipboard.
    if (pmWidget().clipboardPartition() != nullptr && operationStack().findDeviceForPartition(pmWidget().clipboardPartition()) == nullptr)
        pmWidget().setClipboardPartition(nullptr);

    pmWidget().updatePartitions();
    enableActions();
}

void MainWindow::onClearAllOperations()
{
    if (KMessageBox::warningContinueCancel(this,
                                           xi18nc("@info", "Do you really want to clear the list of pending operations?"),
                                           xi18nc("@title:window", "Clear Pending Operations?"),
                                           KGuiItem(xi18nc("@action:button", "Clear Pending Operations"), QStringLiteral("arrow-right")),
                                           KStandardGuiItem::cancel(), QStringLiteral("reallyClearPendingOperations")) == KMessageBox::Continue) {
        Log() << xi18nc("@info:status", "Clearing the list of pending operations.");
        operationStack().clearOperations();

        pmWidget().updatePartitions();
        enableActions();
    }
}

void MainWindow::onCreateNewPartitionTable()
{
    Q_ASSERT(pmWidget().selectedDevice());

    if (pmWidget().selectedDevice() == nullptr) {
        qWarning() << "selected device is null.";
        return;
    }

    QPointer<CreatePartitionTableDialog> dlg = new CreatePartitionTableDialog(this, *pmWidget().selectedDevice());

    if (dlg->exec() == QDialog::Accepted)
        operationStack().push(new CreatePartitionTableOperation(*pmWidget().selectedDevice(), dlg->type()));

    delete dlg;
}

void MainWindow::onImportPartitionTable()
{
    Q_ASSERT(pmWidget().selectedDevice());

    const QUrl url = QFileDialog::getOpenFileUrl(this, QStringLiteral("Import Partition Table"));

    if (url.isEmpty())
        return;

    QFile file;
    if (url.isLocalFile())
    {
        file.setFileName(url.toLocalFile());
        if (!file.open(QFile::ReadOnly))
        {
            KMessageBox::error(this, xi18nc("@info", "Could not open input file <filename>%1</filename> for import", url.toLocalFile()), xi18nc("@title:window", "Error Importing Partition Table"));
            return;
        }

    }
    else
    {
        QTemporaryFile tempFile;

        if (!tempFile.open()) {
            KMessageBox::error(this, xi18nc("@info", "Could not create temporary file when trying to save to <filename>%1</filename>.", tempFile.fileName()), xi18nc("@title:window", "Error Importing Partition Table"));
            return;
        }

        KIO::FileCopyJob *job = KIO::file_copy(url, QUrl::fromLocalFile(tempFile.fileName()));
        KJobWidgets::setWindow(job, this);
        job->exec();
        if (job->error()) {
            KMessageBox::error(this, xi18nc("@info", "Could not open input file <filename>%1</filename> for import: %2", url.fileName(), job->errorString()), xi18nc("@title:window", "Error Importing Partition Table"));
            return;
        }
        file.setFileName(tempFile.fileName());
        if (!file.open(QFile::ReadOnly))
        {
            KMessageBox::error(this, xi18nc("@info", "Could not open temporary file <filename>%1</filename> while trying to import from <filename>%2</filename>.", tempFile.fileName(), url.fileName()), xi18nc("@title:window", "Error Importing Partition Table"));
            return;
        }
    }

    Device& device = *pmWidget().selectedDevice();

    QString line;
    quint32 lineNo = 0;
    bool haveMagic = false;
    PartitionTable* ptable = nullptr;
    PartitionTable::TableType tableType = PartitionTable::unknownTableType;

    while (!(line = QString::fromUtf8(file.readLine())).isEmpty()) {
        lineNo++;
        line = line.trimmed();

        if (line.isEmpty())
            continue;

        QRegularExpression re(QStringLiteral("(\\d+);(\\d+);(\\d+);(\\w+);(\\w+);(\"\\w*\");(\"[^\"]*\")"));
        QRegularExpressionMatch rePartition = re.match(line);
        re.setPattern(QStringLiteral("type:\\s\"(.+)\""));
        QRegularExpressionMatch reType = re.match(line);
        re.setPattern(QStringLiteral("align:\\s\"(cylinder|sector)\""));
        QRegularExpressionMatch reAlign = re.match(line);
        re.setPattern(QStringLiteral("^##|v(\\d+)|##"));
        QRegularExpressionMatch reMagic = re.match(line);

        if (!(haveMagic || reMagic.hasMatch())) {
            KMessageBox::error(this, xi18nc("@info", "The import file <filename>%1</filename> does not contain a valid partition table.", url.fileName()), xi18nc("@title:window", "Error While Importing Partition Table"));
            return;
        } else
            haveMagic = true;

        if (line.startsWith(QStringLiteral("#")))
            continue;

        if (reType.hasMatch()) {
            if (ptable != nullptr) {
                KMessageBox::error(this, xi18nc("@info", "Found more than one partition table type in import file (line %1).", lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            tableType = PartitionTable::nameToTableType(reType.captured(1));

            if (tableType == PartitionTable::unknownTableType) {
                KMessageBox::error(this, xi18nc("@info", "Partition table type \"%1\" is unknown (line %2).", reType.captured(1), lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            if (tableType != PartitionTable::msdos && tableType != PartitionTable::gpt) {
                KMessageBox::error(this, xi18nc("@info", "Partition table type \"%1\" is not supported for import (line %2).", reType.captured(1), lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            ptable = new PartitionTable(tableType, PartitionTable::defaultFirstUsable(device, tableType), PartitionTable::defaultLastUsable(device, tableType));
            operationStack().push(new CreatePartitionTableOperation(device, ptable));
        } else if (reAlign.hasMatch()) {
            // currently ignored
        } else if (rePartition.hasMatch()) {
            if (ptable == nullptr) {
                KMessageBox::error(this, xi18nc("@info", "Found partition but no partition table type (line %1).",  lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            qint32 num = rePartition.captured(1).toInt();
            qint64 firstSector = rePartition.captured(2).toLongLong();
            qint64 lastSector = rePartition.captured(3).toLongLong();
            QLatin1String fsName = QLatin1String(rePartition.captured(4).toLatin1());
            QString roleNames = rePartition.captured(5);
            QString volumeLabel = rePartition.captured(6).replace(QStringLiteral("\""), QString());
            QStringList flags = rePartition.captured(7).replace(QStringLiteral("\""), QString()).split(QStringLiteral(","));

            if (firstSector < ptable->firstUsable() || lastSector > ptable->lastUsable()) {
                KMessageBox::error(this, xi18nc("@info the partition is NOT a device path, just a number", "Partition %1 would be outside the device's boundaries (line %2).", num, lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            if (firstSector >= lastSector) {
                KMessageBox::error(this, xi18nc("@info", "Partition %1 has end before start sector (line %2).", num, lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            PartitionNode* parent = ptable;

            Q_ASSERT(parent);

            PartitionRole role(PartitionRole::None);

            if (roleNames == QStringLiteral("extended"))
                role = PartitionRole(PartitionRole::Extended);
            else if (roleNames == QStringLiteral("logical")) {
                role = PartitionRole(PartitionRole::Logical);
                parent = ptable->findPartitionBySector(firstSector, PartitionRole(PartitionRole::Extended));
            } else if (roleNames == QStringLiteral("primary"))
                role = PartitionRole(PartitionRole::Primary);

            if (role == PartitionRole(PartitionRole::None)) {
                KMessageBox::error(this, xi18nc("@info the partition is NOT a device path, just a number", "Unrecognized partition role \"%1\" for partition %2 (line %3).", roleNames, num, lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            if (parent == nullptr) {
                KMessageBox::error(this, xi18nc("@info the partition is NOT a device path, just a number", "No parent partition or partition table found for partition %1 (line %2).", num, lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            if (role.has(PartitionRole::Extended) && !PartitionTable::tableTypeSupportsExtended(tableType)) {
                KMessageBox::error(this, xi18nc("@info", "The partition table type \"%1\" does not support extended partitions, but one was found (line %2).", PartitionTable::tableTypeToName(tableType), lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            FileSystem* fs = FileSystemFactory::create(FileSystem::typeForName(fsName, { QStringLiteral("C") }), firstSector, lastSector, device.logicalSize());

            if (fs == nullptr) {
                KMessageBox::error(this, xi18nc("@info the partition is NOT a device path, just a number", "Could not create file system \"%1\" for partition %2 (line %3).", fsName, num, lineNo), xi18nc("@title:window", "Error While Importing Partition Table"));
                return;
            }

            if (fs->supportSetLabel() != FileSystem::cmdSupportNone && !volumeLabel.isEmpty())
                fs->setLabel(volumeLabel);

            Partition* p = new Partition(parent, device, role, fs, firstSector, lastSector, QString(), PartitionTable::FlagNone, QString(), false, PartitionTable::FlagNone, Partition::State::New);

            operationStack().push(new NewOperation(device, p));
        } else
            Log(Log::Level::warning) << xi18nc("@info:status", "Could not parse line %1 from import file. Ignoring it.", lineNo);
    }

    if (ptable->type() == PartitionTable::msdos && ptable->isSectorBased(device))
        ptable->setType(device, PartitionTable::msdos_sectorbased);
}

void MainWindow::onExportPartitionTable()
{
    Q_ASSERT(pmWidget().selectedDevice());
    Q_ASSERT(pmWidget().selectedDevice()->partitionTable());

    const QUrl url = QFileDialog::getSaveFileUrl();

    if (url.isEmpty())
        return;

    QTemporaryFile tempFile;

    if (!tempFile.open()) {
        KMessageBox::error(this, xi18nc("@info", "Could not create temporary file when trying to save to <filename>%1</filename>.", url.fileName()), xi18nc("@title:window", "Error Exporting Partition Table"));
        return;
    }

    QTextStream stream(&tempFile);

    stream << "##|v1|## partition table of " << pmWidget().selectedDevice()->deviceNode() << "\n";
    stream << "# on " << QLocale::c().toString(QDateTime::currentDateTime()) << "\n";
    stream << *pmWidget().selectedDevice()->partitionTable();

    tempFile.close();

    KIO::CopyJob* job = KIO::move(QUrl::fromLocalFile(tempFile.fileName()), url, KIO::HideProgressInfo);
    job->exec();
    if (job->error())
        job->uiDelegate()->showErrorMessage();
}

void MainWindow::onCreateNewVolumeGroup()
{
    QString vgName;
    QVector<const Partition*> pvList;
    qint32 peSize = 4;
    // *NOTE*: vgName & pvList will be modified and validated by the dialog
    QPointer<CreateVolumeGroupDialog> dlg = new CreateVolumeGroupDialog(this, vgName, pvList, peSize, operationStack().previewDevices());
    if (dlg->exec() == QDialog::Accepted)
        operationStack().push(new CreateVolumeGroupOperation(vgName, pvList, peSize));

    delete dlg;
}

void MainWindow::onResizeVolumeGroup()
{
    if (pmWidget().selectedDevice()->type() == Device::Type::LVM_Device) {
        LvmDevice* d = dynamic_cast<LvmDevice*>(pmWidget().selectedDevice());

        QVector<const Partition*> pvList;
        // *NOTE*: pvList will be modified and validated by the dialog

        QPointer<ResizeVolumeGroupDialog> dlg = new ResizeVolumeGroupDialog(this, d, pvList);
        if (dlg->exec() == QDialog::Accepted)
            operationStack().push(new ResizeVolumeGroupOperation(*d, pvList));

        delete dlg;
    }
}

void MainWindow::onRemoveVolumeGroup()
{
    Device* tmpDev = pmWidget().selectedDevice();
    if (tmpDev->type() == Device::Type::LVM_Device) {
        operationStack().push(new RemoveVolumeGroupOperation(*(dynamic_cast<LvmDevice*>(tmpDev))));
    }
}

void MainWindow::onDeactivateVolumeGroup()
{
    Device* tmpDev = pmWidget().selectedDevice();
    if (tmpDev->type() == Device::Type::LVM_Device) {
        DeactivateVolumeGroupOperation* deactivate = new DeactivateVolumeGroupOperation( *(dynamic_cast<LvmDevice*>(tmpDev)) );
        Report* tmpReport = new Report(nullptr);
        if (deactivate->execute(*tmpReport)) {
            deactivate->preview();
            actionCollection()->action(QStringLiteral("resizeVolumeGroup"))->setEnabled(false);
            actionCollection()->action(QStringLiteral("deactivateVolumeGroup"))->setEnabled(false);
        }
        delete tmpReport;
        pmWidget().updatePartitions();
        enableActions();
    }
}

void MainWindow::onFileSystemSupport()
{
    FileSystemSupportDialog dlg(this);
    dlg.exec();
}

void MainWindow::onShowAboutKPMcore()
{
    KAboutApplicationDialog dlg(aboutKPMcore(), this);
    dlg.exec();
}

void MainWindow::onSettingsChanged()
{
    if (CoreBackendManager::self()->backend()->id() != Config::backend()) {
        CoreBackendManager::self()->unload();
        // FIXME: if loadBackend() fails to load the configured backend and loads the default
        // one instead it also sets the default backend in the config; the config dialog will
        // overwrite that again, however, after we're done here.
        if (loadBackend()) {
            deviceScanner().setupConnections();
            scanDevices();
            FileSystemFactory::init();
        } else
            close();
    }

    enableActions();
    pmWidget().updatePartitions();

    PartitionAlignment::setSectorAlignment(Config::sectorAlignment());

    emit settingsChanged();
}

void MainWindow::onConfigureOptions()
{
    if (ConfigureOptionsDialog::showDialog(QStringLiteral("Settings")))
        return;

    QPointer<ConfigureOptionsDialog> dlg = new ConfigureOptionsDialog(this, operationStack(), QStringLiteral("Settings"));

    // FIXME: we'd normally use settingsChanged(), according to the kde api docs. however, this
    // is emitted each time the user changes any of our own settings (backend, default file system), without
    // applying or clicking ok. so the below is the workaround for that.
    connect(dlg->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &MainWindow::onSettingsChanged);
    connect(dlg->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &MainWindow::onSettingsChanged);

    dlg->show();
}

void MainWindow::onSmartStatusDevice()
{
    Q_ASSERT(pmWidget().selectedDevice());

    if (pmWidget().selectedDevice()) {
        QPointer<SmartDialog> dlg = new SmartDialog(this, *pmWidget().selectedDevice());
        dlg->exec();
        delete dlg;
    }
}

void MainWindow::onPropertiesDevice(const QString&)
{
    Q_ASSERT(pmWidget().selectedDevice());

    if (pmWidget().selectedDevice()) {
        Device& d = *pmWidget().selectedDevice();

        QPointer<DevicePropsDialog> dlg = new DevicePropsDialog(this, d);
        if (dlg->exec() == QDialog::Accepted) {
            if (d.partitionTable()->type() == PartitionTable::msdos && dlg->sectorBasedAlignment())
                d.partitionTable()->setType(d, PartitionTable::msdos_sectorbased);
            else if (d.partitionTable()->type() == PartitionTable::msdos_sectorbased && dlg->cylinderBasedAlignment())
                d.partitionTable()->setType(d, PartitionTable::msdos);

            on_m_OperationStack_devicesChanged();
            pmWidget().updatePartitions();
        }

        delete dlg;
    }
}

static KLocalizedString checkSupportInNode(const PartitionNode* parent)
{
    if (parent == nullptr)
        return KLocalizedString();

    KLocalizedString rval;

    for (const auto &node : parent->children()) {
        const Partition* p = dynamic_cast<const Partition*>(node);

        if (p == nullptr)
            continue;

        if (node->children().size() > 0 && !rval.isEmpty())
            rval = kxi18n("%1%2").subs(rval).subs(checkSupportInNode(node));
        else
            rval = checkSupportInNode(node);

        if ((!p->fileSystem().supportToolFound() && !p->fileSystem().supportToolName().name.isEmpty()) && !rval.isEmpty())
            rval = kxi18n("%1%2").subs(rval).subs(kxi18n("<tr>"
                                   "<td>%1</td>"
                                   "<td>%2</td>"
                                   "<td>%3</td>"
                                   "<td><link>%4</link></td>"
                                   "</tr>")
                 .subs(p->deviceNode())
                 .subs(p->fileSystem().name())
                 .subs(p->fileSystem().supportToolName().name)
                 .subs(p->fileSystem().supportToolName().url.toString()));
        else if (!p->fileSystem().supportToolFound() && !p->fileSystem().supportToolName().name.isEmpty())
            rval =kxi18n("<tr>"
                                   "<td>%1</td>"
                                   "<td>%2</td>"
                                   "<td>%3</td>"
                                   "<td><link>%4</link></td>"
                                   "</tr>")
                 .subs(p->deviceNode())
                 .subs(p->fileSystem().name())
                 .subs(p->fileSystem().supportToolName().name)
                 .subs(p->fileSystem().supportToolName().url.toString());
    }

    return rval;
}

void MainWindow::checkFileSystemSupport()
{
    KLocalizedString supportList, supportInNode;
    bool missingSupportTools = false;

    const auto previewDevices = operationStack().previewDevices();
    for (auto const &d : previewDevices ) {
        supportInNode = checkSupportInNode(d->partitionTable());
        if (!supportInNode.isEmpty() && !supportList.isEmpty()) {
            missingSupportTools = true;
            supportList = kxi18n("%1%2").subs(supportList).subs(supportInNode);
        }
        else if (!supportInNode.isEmpty()) {
            missingSupportTools = true;
            supportList = supportInNode;
        }
    }

    if (missingSupportTools)
        KMessageBox::information(this,
                                 xi18nc("@info",
                                        "<para>No support tools were found for file systems currently present on hard disks in this computer:</para>"
                                        "<table style='margin-top:12px'>"
                                        "<tr>"
                                        "<td style='font-weight:bold;padding-right:12px;white-space:nowrap;'>Partition</td>"
                                        "<td style='font-weight:bold;padding-right:12px;white-space:nowrap;'>File System</td>"
                                        "<td style='font-weight:bold;padding-right:12px;white-space:nowrap;'>Support Tools</td>"
                                        "<td style='font-weight:bold;padding-right:12px;white-space:nowrap;'>URL</td>"
                                        "</tr>"
                                        "%1"
                                        "</table>"
                                        "<para>As long as the support tools for these file systems are not installed you will not be able to modify them.</para>"
                                        "<para>You should find packages with these support tools in your distribution's package manager.</para>",
                                        supportList),
                                 xi18nc("@title:window", "Missing File System Support Packages"),
                                 QStringLiteral("showInformationOnMissingFileSystemSupport"), KMessageBox::Notify | KMessageBox::AllowLink);
}
