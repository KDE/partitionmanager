/*
    SPDX-FileCopyrightText: 2009-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(PARTITIONMANAGERWIDGET_H)

#define PARTITIONMANAGERWIDGET_H

#include <core/operationstack.h>
#include <core/operationrunner.h>

#include "ui_partitionmanagerwidgetbase.h"

#include <QWidget>

class Partition;
class PartWidget;
class Device;

class QWidget;
class QLabel;
class QPoint;

/** The central widget for the application.

    @author Volker Lanz <vl@fidra.de>
*/
class PartitionManagerWidget : public QWidget, Ui::PartitionManagerWidgetBase
{
    Q_OBJECT
    Q_DISABLE_COPY(PartitionManagerWidget)

public:
    explicit PartitionManagerWidget(QWidget* parent = nullptr);
    virtual ~PartitionManagerWidget();

Q_SIGNALS:
    void selectedPartitionChanged(const Partition*);
    void contextMenuRequested(const QPoint&);
    void deviceDoubleClicked(const Device*);
    void partitionDoubleClicked(const Partition* p);

public:
    void setSelectedDevice(Device* d);
    void setSelectedDevice(const QString& deviceNode);

    void onNewPartition();
    void onResizePartition();
    void onDeletePartition(bool shred = false);
    void onShredPartition();

    void onCopyPartition();
    void onPastePartition();

    void onEditMountPoint();
    void onMountPartition();
    void onDecryptPartition();

    void onCheckPartition();

    void onBackupPartition();
    void onRestorePartition();

    void onPropertiesPartition();

    void init(OperationStack* ostack);

    void clear();

    Device* selectedDevice() {
        return m_SelectedDevice;
    }
    const Device* selectedDevice() const {
        return m_SelectedDevice;
    }

    Partition* selectedPartition();
    void setSelectedPartition(const Partition* p);

    Partition* clipboardPartition() {
        return m_ClipboardPartition;
    }
    const Partition* clipboardPartition() const {
        return m_ClipboardPartition;
    }
    void setClipboardPartition(Partition* p) {
        m_ClipboardPartition = p;
    }

    void updatePartitions();

    // here we expect the full "sda1" or "nvme1n1p1"
    bool setCurrentPartitionByName(const QString& partitionName);

protected:
    OperationStack& operationStack() {
        return *m_OperationStack;
    }
    const OperationStack& operationStack() const {
        return *m_OperationStack;
    }

    void setupConnections();
    void loadConfig();
    void saveConfig() const;
    bool showInsertDialog(Partition& insertPartition, qint64 sourceLength);

    PartTableWidget& partTableWidget() {
        Q_ASSERT(m_PartTableWidget);
        return *m_PartTableWidget;
    }
    const PartTableWidget& partTableWidget() const {
        Q_ASSERT(m_PartTableWidget);
        return *m_PartTableWidget;
    }

    QTreeWidget& treePartitions() {
        Q_ASSERT(m_TreePartitions);
        return *m_TreePartitions;
    }
    const QTreeWidget& treePartitions() const {
        Q_ASSERT(m_TreePartitions);
        return *m_TreePartitions;
    }


    void onHeaderContextMenu(const QPoint& p);

protected Q_SLOTS:
    void on_m_TreePartitions_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void on_m_TreePartitions_customContextMenuRequested(const QPoint& pos);
    void on_m_TreePartitions_itemDoubleClicked(QTreeWidgetItem* item, int);

    void on_m_PartTableWidget_itemSelectionChanged(PartWidget* item);
    void on_m_PartTableWidget_customContextMenuRequested(const QPoint& pos);
    void on_m_PartTableWidget_itemDoubleClicked();

private:
    OperationStack* m_OperationStack;
    Device* m_SelectedDevice;
    Partition* m_ClipboardPartition;
};

#endif

