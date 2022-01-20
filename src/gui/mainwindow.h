/*
    SPDX-FileCopyrightText: 2008-2012 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2019 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(MAINWINDOW_H)

#define MAINWINDOW_H

#include <core/operationrunner.h>
#include <core/operationstack.h>
#include <core/devicescanner.h>

#include "ui_mainwindowbase.h"

#include <KMessageWidget>
#include <KXmlGuiWindow>

class ApplyProgressDialog;
class ScanProgressDialog;
class Device;
class Partition;
class InfoPane;

class KActionCollection;

class QWidget;
class QLabel;
class QCloseEvent;
class QEvent;

/** The application's main window.

    @author Volker Lanz <vl@fidra.de>
*/
class MainWindow : public KXmlGuiWindow, public Ui::MainWindowBase
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    explicit MainWindow(QWidget* parent = nullptr);

    // for instance `/dev/sda`
    void setCurrentDeviceByName(const QString& name);

    void setCurrentPartitionByName(const QString& partitionNumber);

    // forbids the user to select another device.
    // this is used in conjunction with --device
    // rationale is that if the user specifies a device,
    // we can't allow him to select another one by mistake while
    // clicking in the UI.
    void setDisallowOtherDevices();

    // disallowOtherDevices hides the DockWidget, but unfortunately
    // this is saved by the Window State when restored, even if we
    // are not disallowing it this time. At the same time the user
    // could have hidden it, so we need to restore only if hidden
    // just by the call to disallowOtherDevices().
    void showDevicePanelIfPreviouslyHiddenByDisallowOtherDevices();

Q_SIGNALS:
    void settingsChanged();
    void scanFinished();

protected:
    void init();
    void setupObjectNames();
    void setupActions();
    void setupConnections();
    void setupStatusBar();
    void loadConfig();
    void saveConfig() const;
    void updateWindowTitle();
    void updateSeletedDeviceMenu();
    void checkFileSystemSupport();

    void enableActions();

    void closeEvent(QCloseEvent*) override;
    void changeEvent(QEvent* event) override;

    void setSavedSelectedDeviceNode(const QString& s) {
        m_SavedSelectedDeviceNode = s;
    }
    const QString& savedSelectedDeviceNode() const {
        return m_SavedSelectedDeviceNode;
    }

    InfoPane& infoPane() {
        Q_ASSERT(m_InfoPane);
        return *m_InfoPane;
    }

    PartitionManagerWidget& pmWidget() {
        Q_ASSERT(m_PartitionManagerWidget);
        return *m_PartitionManagerWidget;
    }
    const PartitionManagerWidget& pmWidget() const {
        Q_ASSERT(m_PartitionManagerWidget);
        return *m_PartitionManagerWidget;
    }

    ListDevices& listDevices() {
        Q_ASSERT(m_ListDevices);
        return *m_ListDevices;
    }
    const ListDevices& listDevices() const {
        Q_ASSERT(m_ListDevices);
        return *m_ListDevices;
    }

    ListOperations& listOperations() {
        Q_ASSERT(m_ListOperations);
        return *m_ListOperations;
    }
    const ListOperations& listOperations() const {
        Q_ASSERT(m_ListOperations);
        return *m_ListOperations;
    }

    TreeLog& treeLog() {
        Q_ASSERT(m_TreeLog);
        return *m_TreeLog;
    }
    const TreeLog& treeLog() const {
        Q_ASSERT(m_TreeLog);
        return *m_TreeLog;
    }

    QDockWidget& dockInformation() {
        Q_ASSERT(m_DockInformation);
        return *m_DockInformation;
    }
    const QDockWidget& dockInformation() const {
        Q_ASSERT(m_DockInformation);
        return *m_DockInformation;
    }

    QDockWidget& dockDevices() {
        Q_ASSERT(m_DockDevices);
        return *m_DockDevices;
    }
    const QDockWidget& dockDevices() const {
        Q_ASSERT(m_DockDevices);
        return *m_DockDevices;
    }

    QDockWidget& dockOperations() {
        Q_ASSERT(m_DockOperations);
        return *m_DockOperations;
    }
    const QDockWidget& dockOperations() const {
        Q_ASSERT(m_DockOperations);
        return *m_DockOperations;
    }

    QDockWidget& dockLog() {
        Q_ASSERT(m_DockLog);
        return *m_DockLog;
    }
    const QDockWidget& dockLog() const {
        Q_ASSERT(m_DockLog);
        return *m_DockLog;
    }

    QLabel& statusText() {
        Q_ASSERT(m_StatusText);
        return *m_StatusText;
    }
    const QLabel& statusText() const {
        Q_ASSERT(m_StatusText);
        return *m_StatusText;
    }

    OperationStack& operationStack() {
        Q_ASSERT(m_OperationStack);
        return *m_OperationStack;
    }
    const OperationStack& operationStack() const {
        Q_ASSERT(m_OperationStack);
        return *m_OperationStack;
    }

    OperationRunner& operationRunner() {
        Q_ASSERT(m_OperationRunner);
        return *m_OperationRunner;
    }
    const OperationRunner& operationRunner() const {
        Q_ASSERT(m_OperationRunner);
        return *m_OperationRunner;
    }

    DeviceScanner& deviceScanner() {
        Q_ASSERT(m_DeviceScanner);
        return *m_DeviceScanner;
    }
    const DeviceScanner& deviceScanner() const {
        Q_ASSERT(m_DeviceScanner);
        return *m_DeviceScanner;
    }

    ApplyProgressDialog& applyProgressDialog() {
        Q_ASSERT(m_ApplyProgressDialog);
        return *m_ApplyProgressDialog;
    }
    const ApplyProgressDialog& applyProgressDialog() const {
        Q_ASSERT(m_ApplyProgressDialog);
        return *m_ApplyProgressDialog;
    }

    ScanProgressDialog& scanProgressDialog() {
        Q_ASSERT(m_ScanProgressDialog);
        return *m_ScanProgressDialog;
    }
    const ScanProgressDialog& scanProgressDialog() const {
        Q_ASSERT(m_ScanProgressDialog);
        return *m_ScanProgressDialog;
    }

    KMessageWidget &MessageWidget() {
        Q_ASSERT(m_MessageWidget);
        return *m_MessageWidget;
    }
    const KMessageWidget &MessageWidget() const {
        Q_ASSERT(m_MessageWidget);
        return *m_MessageWidget;
    }

    void onSelectedDeviceMenuTriggered(bool);

protected Q_SLOTS:
    void on_m_PartitionManagerWidget_selectedPartitionChanged(const Partition* p);
    void on_m_PartitionManagerWidget_contextMenuRequested(const QPoint& pos);
    void on_m_PartitionManagerWidget_deviceDoubleClicked(const Device*);
    void on_m_PartitionManagerWidget_partitionDoubleClicked(const Partition*);

    void on_m_DockInformation_dockLocationChanged(Qt::DockWidgetArea);

    void on_m_OperationStack_operationsChanged();
    void on_m_OperationStack_devicesChanged();

    void on_m_DeviceScanner_finished();
    void on_m_DeviceScanner_progress(const QString& device_node, int percent);

    void on_m_ApplyProgressDialog_finished();

    void listDevicesContextMenuRequested(const QPoint& pos);
    void on_m_ListDevices_selectionChanged(const QString& device_node);

    void treeLogContextMenuRequested(const QPoint& pos);
    void listOperationsContextMenuRequested(const QPoint& pos);

protected:
    void scanDevices();

    void onRefreshDevices();
    void onCreateNewPartitionTable();
    void onCreateNewVolumeGroup();
    void onRemoveVolumeGroup();
    void onResizeVolumeGroup();
    void onDeactivateVolumeGroup();
    void onExportPartitionTable();
    void onImportPartitionTable();

    void onApplyAllOperations();
    void onUndoOperation();
    void onClearAllOperations();

    void onConfigureOptions();
    void onSettingsChanged();

    void onFileSystemSupport();
    void onShowAboutKPMcore();

    void onSmartStatusDevice();
    void onPropertiesDevice(const QString& deviceNode = {});

private:
    QMenu* createPopupMenu() override;

    void askForPermissions();

Q_SIGNALS:
    void showMessageWidget();
    void hideMessageWidget();

private:
    OperationStack* m_OperationStack;
    OperationRunner* m_OperationRunner;
    DeviceScanner* m_DeviceScanner;
    ApplyProgressDialog* m_ApplyProgressDialog;
    ScanProgressDialog* m_ScanProgressDialog;
    QLabel* m_StatusText;
    QString m_SavedSelectedDeviceNode;

    bool m_permissionGranted;
};

#endif
