/*
    SPDX-FileCopyrightText: 2008-2012 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015-2016 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Abhijeet Sharma <sharma.abhijeet2096@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/partitionmanagerwidget.h"
#include "gui/partpropsdialog.h"
#include "gui/resizedialog.h"
#include "gui/newdialog.h"
#include "gui/applyprogressdialog.h"
#include "gui/insertdialog.h"
#include "gui/editmountpointdialog.h"
#include "util/guihelpers.h"

#include <core/partition.h>
#include <core/device.h>
#include <core/operationstack.h>
#include <core/partitiontable.h>

#include <fs/filesystemfactory.h>
#include <fs/luks.h>

#include <gui/partwidget.h>

#include <ops/deleteoperation.h>
#include <ops/resizeoperation.h>
#include <ops/newoperation.h>
#include <ops/copyoperation.h>
#include <ops/checkoperation.h>
#include <ops/backupoperation.h>
#include <ops/restoreoperation.h>
#include <ops/setfilesystemlabeloperation.h>
#include <ops/setpartflagsoperation.h>
#include <ops/createfilesystemoperation.h>

#include <util/globallog.h>
#include <util/capacity.h>
#include <util/report.h>
#include <util/helpers.h>

#include <QFileDialog>
#include <QLocale>
#include <QPointer>
#include <QReadLocker>

#include <KLocalizedString>
#include <KMessageBox>

#include <config.h>

#include <typeinfo>

class PartitionTreeWidgetItem : public QTreeWidgetItem
{
    Q_DISABLE_COPY(PartitionTreeWidgetItem)

public:
    PartitionTreeWidgetItem(const Partition* p) : QTreeWidgetItem(), m_Partition(p) {}
    const Partition* partition() const {
        return m_Partition;
    }

private:
    const Partition* m_Partition;
};

/** Creates a new PartitionManagerWidget instance.
    @param parent the parent widget
*/
PartitionManagerWidget::PartitionManagerWidget(QWidget* parent) :
    QWidget(parent),
    Ui::PartitionManagerWidgetBase(),
    m_OperationStack(nullptr),
    m_SelectedDevice(nullptr),
    m_ClipboardPartition(nullptr)
{
    setupUi(this);

    treePartitions().header()->setStretchLastSection(false);
    treePartitions().header()->setContextMenuPolicy(Qt::CustomContextMenu);
    treePartitions().setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::TopEdge}));
}

PartitionManagerWidget::~PartitionManagerWidget()
{
    saveConfig();
}

void PartitionManagerWidget::init(OperationStack* ostack)
{
    m_OperationStack = ostack;

    loadConfig();
    setupConnections();
}

void PartitionManagerWidget::loadConfig()
{
    QList<int> colWidths = Config::treePartitionColumnWidths();
    QList<int> colPositions = Config::treePartitionColumnPositions();
    QList<int> colVisible = Config::treePartitionColumnVisible();
    QHeaderView* header = treePartitions().header();

    for (int i = 0; i < treePartitions().columnCount(); i++) {
        if (colPositions[0] != -1 && colPositions.size() > i)
            header->moveSection(header->visualIndex(i), colPositions[i]);

        if (colVisible[0] != -1 && colVisible.size() > i)
            treePartitions().setColumnHidden(i, colVisible[i] == 0);

        if (colWidths[0] != -1 && colWidths.size() > i)
            treePartitions().setColumnWidth(i, colWidths[i]);
    }
}

void PartitionManagerWidget::saveConfig() const
{
    QList<int> colWidths;
    QList<int> colPositions;
    QList<int> colVisible;

    for (int i = 0; i < treePartitions().columnCount(); i++) {
        colPositions.append(treePartitions().header()->visualIndex(i));
        colVisible.append(treePartitions().isColumnHidden(i) ? 0 : 1);
        colWidths.append(treePartitions().columnWidth(i));
    }

    Config::setTreePartitionColumnPositions(colPositions);
    Config::setTreePartitionColumnVisible(colVisible);
    Config::setTreePartitionColumnWidths(colWidths);

    Config::self()->save();
}

void PartitionManagerWidget::setupConnections()
{
    connect(treePartitions().header(), &QHeaderView::customContextMenuRequested, this, &PartitionManagerWidget::onHeaderContextMenu);
}

void PartitionManagerWidget::clear()
{
    setSelectedDevice(nullptr);
    setClipboardPartition(nullptr);
    treePartitions().clear();
    partTableWidget().clear();
}

void PartitionManagerWidget::setSelectedPartition(const Partition* p)
{
    if (p == nullptr) {
        treePartitions().setCurrentItem(nullptr);
        Q_EMIT selectedPartitionChanged(nullptr);
        updatePartitions();
    } else
        partTableWidget().setActivePartition(p);
}

Partition* PartitionManagerWidget::selectedPartition()
{
    if (selectedDevice() == nullptr || selectedDevice()->partitionTable() == nullptr || partTableWidget().activeWidget() == nullptr)
        return nullptr;

    return partTableWidget().activeWidget()->partition();
}

void PartitionManagerWidget::setSelectedDevice(const QString& deviceNode)
{
    QReadLocker lockDevices(&operationStack().lock());

    const auto previewDevices = operationStack().previewDevices();
    for (const auto &d : previewDevices) {
        if (d->deviceNode() == deviceNode) {
            setSelectedDevice(d);
            return;
        }
    }

    setSelectedDevice(nullptr);
}

void PartitionManagerWidget::setSelectedDevice(Device* d)
{
    m_SelectedDevice = d;
    setSelectedPartition(nullptr);
}

static QTreeWidgetItem* createTreeWidgetItem(const Partition& p)
{
    QTreeWidgetItem* item = new PartitionTreeWidgetItem(&p);

    int i = 0;
    item->setText(i++, p.deviceNode());

    if (p.roles().has(PartitionRole::Luks) && (p.fileSystem().name() != p.fileSystem().nameForType(FileSystem::Type::Luks) && p.fileSystem().name() != p.fileSystem().nameForType(FileSystem::Type::Luks2)))
        item->setText(i, xi18nc("@item:intable Encrypted file systems, e.g. btrfs[Encrypted]", "%1 [Encrypted]", p.fileSystem().name()));
    else
        item->setText(i, p.fileSystem().name());
    item->setIcon(i, createFileSystemColor(p.fileSystem().type(), 14));
    i++;

    item->setText(i, p.mountPoint());
    if (p.isMounted())
        item->setIcon(i, QIcon::fromTheme(QStringLiteral("object-locked")));
    i++;

    item->setText(i++, p.fileSystem().label());
    item->setText(i++, p.fileSystem().uuid());
    item->setText(i++, p.label());
    item->setText(i++, p.uuid());

    item->setText(i++, Capacity::formatByteSize(p.capacity()));
    item->setText(i++, Capacity::formatByteSize(p.used()));
    item->setText(i++, Capacity::formatByteSize(p.available()));

    item->setText(i++, QLocale().toString(p.firstSector()));
    item->setText(i++, QLocale().toString(p.lastSector()));
    item->setText(i++, QLocale().toString(p.length()));

    item->setText(i++, PartitionTable::flagNames(p.activeFlags()).join(QStringLiteral(", ")));

    item->setSizeHint(0, QSize(0, 32));

    return item;
}

void PartitionManagerWidget::updatePartitions()
{
    if (selectedDevice() == nullptr)
        return;

    treePartitions().clear();
    partTableWidget().clear();

    partTableWidget().setPartitionTable(selectedDevice()->partitionTable());

    QTreeWidgetItem* deviceItem = new QTreeWidgetItem();

    QFont font;
    font.setBold(true);
    font.setWeight(QFont::Bold);
    deviceItem->setFont(0, font);

    deviceItem->setText(0, selectedDevice()->prettyName());
    deviceItem->setIcon(0, QIcon::fromTheme(selectedDevice()->iconName()));

    deviceItem->setSizeHint(0, QSize(0, 32));

    treePartitions().addTopLevelItem(deviceItem);

    if (selectedDevice()->partitionTable() != nullptr) {
        const auto children = selectedDevice()->partitionTable()->children();
        for (const auto * p : children) {
            QTreeWidgetItem* item = createTreeWidgetItem(*p);

            for (const auto &child : p->children()) {
                QTreeWidgetItem* childItem = createTreeWidgetItem(*child);
                item->addChild(childItem);
            }

            deviceItem->addChild(item);
            item->setExpanded(true);
        }
    }

    deviceItem->setFirstColumnSpanned(true);
    deviceItem->setExpanded(true);
    deviceItem->setFlags(Qt::ItemIsEnabled);

    partTableWidget().update();
}

void PartitionManagerWidget::on_m_TreePartitions_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem*)
{
    if (current) {
        const PartitionTreeWidgetItem* ptwItem = dynamic_cast<PartitionTreeWidgetItem*>(current);
        partTableWidget().setActivePartition(ptwItem ? ptwItem->partition() : nullptr);
    } else
        partTableWidget().setActiveWidget(nullptr);
}

bool PartitionManagerWidget::setCurrentPartitionByName(const QString& name)
{
    auto rootNode = treePartitions().invisibleRootItem();
    for (int i = 0; i < rootNode->childCount(); i++) {
        auto driveNode = rootNode->child(i);
        for (int e = 0; e < driveNode->childCount(); e++) {
            auto partitionNode = driveNode->child(e);
            const QString text = partitionNode->data(0, Qt::DisplayRole).toString();
            if (text.endsWith(name)) {
                partitionNode->setSelected(true);
                treePartitions().setCurrentItem(partitionNode);
                return true;
            }
        }
    }

    return false;
}

void PartitionManagerWidget::on_m_TreePartitions_itemDoubleClicked(QTreeWidgetItem* item, int)
{
    if (item == treePartitions().topLevelItem(0)) {
        if (selectedDevice() != nullptr)
            Q_EMIT deviceDoubleClicked(selectedDevice());
    } else {
        if (selectedPartition() != nullptr)
            Q_EMIT partitionDoubleClicked(selectedPartition());
    }
}

void PartitionManagerWidget::onHeaderContextMenu(const QPoint& p)
{
    showColumnsContextMenu(p, treePartitions());
}

void PartitionManagerWidget::on_m_PartTableWidget_itemSelectionChanged(PartWidget* item)
{
    if (item == nullptr) {
        treePartitions().setCurrentItem(nullptr);
        Q_EMIT selectedPartitionChanged(nullptr);
        return;
    }

    const Partition* p = item->partition();

    Q_ASSERT(p);

    if (p) {
        QList<QTreeWidgetItem*> findResult = treePartitions().findItems(p->deviceNode(), Qt::MatchFixedString | Qt::MatchRecursive, 0);

        for (const auto &treeWidgetItem : findResult) {
            const PartitionTreeWidgetItem* ptwItem = dynamic_cast<PartitionTreeWidgetItem*>(treeWidgetItem);

            if (ptwItem && ptwItem->partition() == p) {
                treePartitions().setCurrentItem(treeWidgetItem);
                break;
            }
        }
    }

    Q_EMIT selectedPartitionChanged(p);
}

void PartitionManagerWidget::on_m_PartTableWidget_customContextMenuRequested(const QPoint& pos)
{
    Q_EMIT contextMenuRequested(partTableWidget().mapToGlobal(pos));
}

void PartitionManagerWidget::on_m_PartTableWidget_itemDoubleClicked()
{
    if (selectedPartition())
        Q_EMIT partitionDoubleClicked(selectedPartition());
}

void PartitionManagerWidget::on_m_TreePartitions_customContextMenuRequested(const QPoint& pos)
{
    Q_EMIT contextMenuRequested(treePartitions().viewport()->mapToGlobal(pos));
}

void PartitionManagerWidget::onPropertiesPartition()
{
    if (selectedPartition()) {
        Partition& p = *selectedPartition();

        Q_ASSERT(selectedDevice());

        QPointer<PartPropsDialog> dlg = new PartPropsDialog(this, *selectedDevice(), p);

        if (dlg->exec() == QDialog::Accepted) {
            if (dlg->newFileSystemType() != p.fileSystem().type() || dlg->forceRecreate())
                operationStack().push(new CreateFileSystemOperation(*selectedDevice(), p, dlg->newFileSystemType()));

            if (dlg->newLabel() != p.fileSystem().label())
                operationStack().push(new SetFileSystemLabelOperation(p, dlg->newLabel()));

            if (dlg->newFlags() != p.activeFlags())
                operationStack().push(new SetPartFlagsOperation(*selectedDevice(), p, dlg->newFlags()));
        }

        delete dlg;
    }
}

void PartitionManagerWidget::onMountPartition()
{
    Partition* p = selectedPartition();

    Q_ASSERT(p);

    if (p == nullptr) {
        qWarning() << "no partition selected";
        return;
    }

    Report report(nullptr);

    if (p->canMount()) {
        if (!p->mount(report))
            KMessageBox::detailedError(this, xi18nc("@info", "The file system on partition <filename>%1</filename> could not be mounted.", p->deviceNode()), QStringLiteral("<pre>%1</pre>").arg(report.toText()), xi18nc("@title:window", "Could Not Mount File System."));
    } else if (p->canUnmount()) {
        if (!p->unmount(report))
            KMessageBox::detailedError(this, xi18nc("@info", "The file system on partition <filename>%1</filename> could not be unmounted.", p->deviceNode()), QStringLiteral("<pre>%1</pre>").arg(report.toText()), xi18nc("@title:window", "Could Not Unmount File System."));
    }

    if (p->roles().has(PartitionRole::Logical)) {
        Partition* parent = dynamic_cast<Partition*>(p->parent());

        Q_ASSERT(parent);

        if (parent != nullptr)
            parent->checkChildrenMounted();
        else
            qWarning() << "parent is null";
    }

    updatePartitions();
}

void PartitionManagerWidget::onDecryptPartition()
{
    Partition* p = selectedPartition();

    Q_ASSERT(p);

    if (p == nullptr) {
        qWarning() << "no partition selected";
        return;
    }

    if (!p->roles().has(PartitionRole::Luks))
        return;

    const FileSystem& fsRef = p->fileSystem();
    FS::luks* luksFs = const_cast<FS::luks*>(dynamic_cast<const FS::luks*>(&fsRef));
    if (!luksFs)
        return;

    if (luksFs->canCryptOpen(p->partitionPath())) {
        if (!luksFs->cryptOpen(this, p->partitionPath()))
            KMessageBox::detailedError(this,
                                       xi18nc("@info",
                                              "The encrypted file system on partition "
                                              "<filename>%1</filename> could not be "
                                              "unlocked.",
                                              p->deviceNode()),
                                       QString(),
                                       xi18nc("@title:window",
                                             "Could Not Unlock Encrypted File System."));
    } else if (luksFs->canCryptClose(p->partitionPath())) {
        if (!luksFs->cryptClose(p->partitionPath()))
            KMessageBox::detailedError(this,
                                       xi18nc("@info",
                                              "The encrypted file system on partition "
                                              "<filename>%1</filename> could not be "
                                              "locked.",
                                              p->deviceNode()),
                                       QString(),
                                       xi18nc("@title:window",
                                             "Could Not Lock Encrypted File System."));
    }

    updatePartitions();
}

void PartitionManagerWidget::onEditMountPoint()
{
    Partition* p = selectedPartition();

    Q_ASSERT(p);

    if (p == nullptr)
        return;

    QPointer<EditMountPointDialog> dlg = new EditMountPointDialog(this, *p);

    if (dlg->exec() == QDialog::Accepted)
        updatePartitions();

    delete dlg;
}

static bool checkTooManyPartitions(QWidget* parent, const Device& d, const Partition& p)
{
    Q_ASSERT(d.partitionTable());

    if (p.roles().has(PartitionRole::Unallocated) && d.partitionTable()->numPrimaries() >= d.partitionTable()->maxPrimaries() && !p.roles().has(PartitionRole::Logical)) {
        KMessageBox::error(parent, xi18ncp("@info",
                                           "<para>There is already one primary partition on this device. This is the maximum number its partition table type can handle.</para>"
                                           "<para>You cannot create, paste or restore a primary partition on it before you delete an existing one.</para>",
                                           "<para>There are already %1 primary partitions on this device. This is the maximum number its partition table type can handle.</para>"
                                           "<para>You cannot create, paste or restore a primary partition on it before you delete an existing one.</para>",
                                           d.partitionTable()->numPrimaries()), xi18nc("@title:window", "Too Many Primary Partitions."));
        return true;
    }

    return false;
}

void PartitionManagerWidget::onNewPartition()
{
    Q_ASSERT(selectedDevice());
    Q_ASSERT(selectedPartition());

    if (selectedDevice() == nullptr || selectedPartition() == nullptr) {
        qWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
        return;
    }

    Q_ASSERT(selectedDevice()->partitionTable());

    if (selectedDevice()->partitionTable() == nullptr) {
        qWarning() << "partition table on selected device is null";
        return;
    }

    if (checkTooManyPartitions(this, *selectedDevice(), *selectedPartition()))
        return;

    Partition* newPartition = NewOperation::createNew(*selectedPartition(), static_cast<FileSystem::Type>(Config::defaultFileSystem()));

    QPointer<NewDialog> dlg = new NewDialog(this, *selectedDevice(), *newPartition, selectedDevice()->partitionTable()->childRoles(*selectedPartition()));
    if (dlg->exec() == QDialog::Accepted) {
        if (dlg->useUnsecuredPartition()) {
            newPartition->fileSystem().setPosixPermissions(QStringLiteral("777"));
        }
        operationStack().push(new NewOperation(*selectedDevice(), newPartition));
    } else {
        delete newPartition;
    }

    delete dlg;
}

void PartitionManagerWidget::onDeletePartition(bool shred)
{
    Q_ASSERT(selectedDevice());
    Q_ASSERT(selectedPartition());

    if (selectedDevice() == nullptr || selectedPartition() == nullptr) {
        qWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
        return;
    }

    if (selectedPartition()->roles().has(PartitionRole::Logical)) {
        Q_ASSERT(selectedPartition()->parent());

        if (selectedPartition()->parent() == nullptr) {
            qWarning() << "parent of selected partition is null.";
            return;
        }

        if (selectedPartition()->number() > 0 && selectedPartition()->parent()->highestMountedChild() > selectedPartition()->number()) {
            KMessageBox::error(this,
                               xi18nc("@info",
                                      "<para>The partition <filename>%1</filename> cannot currently be deleted because one or more partitions with higher logical numbers are still mounted.</para>"
                                      "<para>Please unmount all partitions with higher logical numbers than %2 first.</para>",
                                      selectedPartition()->deviceNode(), selectedPartition()->number()),
                               xi18nc("@title:window", "Cannot Delete Partition."));

            return;
        }
    }

    if (clipboardPartition() == selectedPartition()) {
        if (KMessageBox::warningContinueCancel(this,
                                               xi18nc("@info",
                                                       "Do you really want to delete the partition that is currently in the clipboard? "
                                                       "It will no longer be available for pasting after it has been deleted."),
                                               xi18nc("@title:window", "Really Delete Partition in the Clipboard?"),
                                               KGuiItem(xi18nc("@action:button", "Delete It"), QStringLiteral("arrow-right")),
                                               KStandardGuiItem::cancel(), QStringLiteral("reallyDeleteClipboardPartition")) == KMessageBox::Cancel)
            return;

        setClipboardPartition(nullptr);
    }

    if (shred && Config::shredSource() == Config::EnumShredSource::random)
        operationStack().push(new DeleteOperation(*selectedDevice(), selectedPartition(), DeleteOperation::ShredAction::RandomShred));
    else if (shred && Config::shredSource() == Config::EnumShredSource::zeros)
        operationStack().push(new DeleteOperation(*selectedDevice(), selectedPartition(), DeleteOperation::ShredAction::ZeroShred));
    else
        operationStack().push(new DeleteOperation(*selectedDevice(), selectedPartition(), DeleteOperation::ShredAction::NoShred));
}

void PartitionManagerWidget::onShredPartition()
{
    onDeletePartition(true);
}

void PartitionManagerWidget::onResizePartition()
{
    Q_ASSERT(selectedDevice());
    Q_ASSERT(selectedPartition());

    if (selectedDevice() == nullptr || selectedPartition() == nullptr) {
        qWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
        return;
    }

    Q_ASSERT(selectedDevice()->partitionTable());

    if (selectedDevice()->partitionTable() == nullptr) {
        qWarning() << "partition table on selected device is null";
        return;
    }

    // we cannot work with selectedPartition() here because opening and closing the dialog will
    // clear the selection, so we'll lose the partition after the dialog's been exec'd
    Partition& p = *selectedPartition();

    qint64 freeBefore = selectedDevice()->partitionTable()->freeSectorsBefore(p);
    qint64 freeAfter = selectedDevice()->partitionTable()->freeSectorsAfter(p);

    if (selectedDevice()->type() == Device::Type::LVM_Device) {
        freeBefore = 0;
        freeAfter  = selectedDevice()->partitionTable()->freeSectors();
    }

    QPointer<ResizeDialog> dlg = new ResizeDialog(this, *selectedDevice(), p, p.firstSector() - freeBefore, p.lastSector() + freeAfter);

    if (dlg->exec() == QDialog::Accepted) {
        if (dlg->resizedFirstSector() == p.firstSector() && dlg->resizedLastSector() == p.lastSector())
            Log(Log::Level::information) << xi18nc("@info:status", "Partition <filename>%1</filename> has the same position and size after resize/move. Ignoring operation.", p.deviceNode());
        else
            operationStack().push(new ResizeOperation(*selectedDevice(), p, dlg->resizedFirstSector(), dlg->resizedLastSector()));
    }

    if (p.roles().has(PartitionRole::Extended)) {
        // Even if the user dismissed the resize dialog we must update the partitions
        // if it's an extended partition:
        // The dialog has to remove and create unallocated children if the user resizes
        // an extended partition. We can't know if that has happened, so to avoid
        // any problems (like, the user resized an extended and then canceled, which would
        // lead to the unallocated children having the wrong size) do this now.
        updatePartitions();
    }

    delete dlg;
}

void PartitionManagerWidget::onCopyPartition()
{
    Q_ASSERT(selectedPartition());

    if (selectedPartition() == nullptr) {
        qWarning() << "selected partition: " << selectedPartition();
        return;
    }

    setClipboardPartition(selectedPartition());
    Log() << xi18nc("@info:status", "Partition <filename>%1</filename> has been copied to the clipboard.", selectedPartition()->deviceNode());
}

void PartitionManagerWidget::onPastePartition()
{
    Q_ASSERT(selectedDevice());
    Q_ASSERT(selectedPartition());

    if (selectedDevice() == nullptr || selectedPartition() == nullptr) {
        qWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
        return;
    }

    if (clipboardPartition() == nullptr) {
        qWarning() << "no partition in the clipboard.";
        return;
    }

    if (checkTooManyPartitions(this, *selectedDevice(), *selectedPartition()))
        return;

    Device* dSource = operationStack().findDeviceForPartition(clipboardPartition());

    Q_ASSERT(dSource);

    if (dSource == nullptr) {
        qWarning() << "source partition is null.";
        return;
    }

    Partition* copiedPartition = CopyOperation::createCopy(*selectedPartition(), *clipboardPartition());

    if (showInsertDialog(*copiedPartition, clipboardPartition()->length()))
        operationStack().push(new CopyOperation(*selectedDevice(), copiedPartition, *dSource, clipboardPartition()));
    else
        delete copiedPartition;
}

bool PartitionManagerWidget::showInsertDialog(Partition& insertedPartition, qint64 sourceLength)
{
    Q_ASSERT(selectedDevice());
    Q_ASSERT(selectedPartition());

    if (selectedDevice() == nullptr || selectedPartition() == nullptr) {
        qWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
        return false;
    }

    const bool overwrite = !selectedPartition()->roles().has(PartitionRole::Unallocated);

    // Make sure the inserted partition has the right parent and logical or primary set. Only then
    // can PartitionTable::alignPartition() work correctly.
    selectedPartition()->parent()->reparent(insertedPartition);

    if (!overwrite) {
        QPointer<InsertDialog> dlg = new InsertDialog(this, *selectedDevice(), insertedPartition, *selectedPartition());

        int result = dlg->exec();
        delete dlg;

        if (result != QDialog::Accepted)
            return false;
    } else if (KMessageBox::warningContinueCancel(this,
               xi18nc("@info", "<para><warning>You are about to lose all data on partition "
                      "<filename>%1</filename>.</warning></para>"
                      "<para>Overwriting one partition with another (or with an image file) will "
                      "destroy all data on this target partition.</para>"
                      "<para>If you continue now and apply the resulting operation in the main "
                      "window, all data currently stored on <filename>%1</filename> will "
                      "unrecoverably be overwritten.</para>",
                      selectedPartition()->deviceNode()),
               xi18nc("@title:window", "Really Overwrite Existing Partition?"),
               KGuiItem(xi18nc("@action:button", "Overwrite Partition"), QStringLiteral("arrow-right")),
               KStandardGuiItem::cancel(),
               QStringLiteral("reallyOverwriteExistingPartition")) == KMessageBox::Cancel)
        return false;

    if (insertedPartition.length() < sourceLength) {
        if (overwrite)
            KMessageBox::error(this, xi18nc("@info",
                                            "<para>The selected partition is not large enough to hold the source partition or the backup file.</para>"
                                            "<para>Pick another target or resize this partition so it is as large as the source.</para>"), xi18nc("@title:window", "Target Not Large Enough"));
        else
            KMessageBox::error(this, xi18nc("@info",
                                            "<para>It is not possible to create the target partition large enough to hold the source.</para>"
                                            "<para>This may happen if not all partitions on a device are correctly aligned "
                                            "or when copying a primary partition into an extended partition.</para>"),
                               xi18nc("@title:window", "Cannot Create Target Partition."));
        return false;
    }

    return true;
}

void PartitionManagerWidget::onCheckPartition()
{
    Q_ASSERT(selectedDevice());
    Q_ASSERT(selectedPartition());

    if (selectedDevice() == nullptr || selectedPartition() == nullptr) {
        qWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
        return;
    }

    operationStack().push(new CheckOperation(*selectedDevice(), *selectedPartition()));
}

void PartitionManagerWidget::onBackupPartition()
{
    Q_ASSERT(selectedDevice());
    Q_ASSERT(selectedPartition());

    if (selectedDevice() == nullptr || selectedPartition() == nullptr) {
        qWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this);

    if (fileName.isEmpty())
        return;

    operationStack().push(new BackupOperation(*selectedDevice(), *selectedPartition(), fileName));
}

void PartitionManagerWidget::onRestorePartition()
{
    Q_ASSERT(selectedDevice());
    Q_ASSERT(selectedPartition());

    if (selectedDevice() == nullptr || selectedPartition() == nullptr) {
        qWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
        return;
    }

    if (checkTooManyPartitions(this, *selectedDevice(), *selectedPartition()))
        return;

    QString fileName = QFileDialog::getOpenFileName(this);
//  QString fileName = "/tmp/backuptest.img";

    if (!fileName.isEmpty() && QFile::exists(fileName)) {
        Partition* restorePartition = RestoreOperation::createRestorePartition(*selectedDevice(), *selectedPartition()->parent(), selectedPartition()->firstSector(), fileName);

        if (restorePartition->length() > selectedPartition()->length()) {
            KMessageBox::error(this, xi18nc("@info", "The file system in the image file <filename>%1</filename> is too large to be restored to the selected partition.", fileName), xi18nc("@title:window", "Not Enough Space to Restore File System."));
            delete restorePartition;
            return;
        }

        if (showInsertDialog(*restorePartition, restorePartition->length()))
            operationStack().push(new RestoreOperation(*selectedDevice(), restorePartition, fileName));
        else
            delete restorePartition;
    }
}

#include "moc_partitionmanagerwidget.cpp"
