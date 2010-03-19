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

#include "gui/partitionmanagerwidget.h"
#include "gui/partwidget.h"
#include "gui/partpropsdialog.h"
#include "gui/resizedialog.h"
#include "gui/newdialog.h"
#include "gui/applyprogressdialog.h"
#include "gui/insertdialog.h"
#include "gui/editmountpointdialog.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/operationstack.h"
#include "core/partitiontable.h"

#include "fs/filesystemfactory.h"

#include "ops/deleteoperation.h"
#include "ops/resizeoperation.h"
#include "ops/newoperation.h"
#include "ops/copyoperation.h"
#include "ops/checkoperation.h"
#include "ops/backupoperation.h"
#include "ops/restoreoperation.h"
#include "ops/setfilesystemlabeloperation.h"
#include "ops/setpartflagsoperation.h"
#include "ops/createfilesystemoperation.h"

#include "util/globallog.h"
#include "util/capacity.h"
#include "util/report.h"
#include "util/helpers.h"

#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kglobal.h>

#include <QCursor>
#include <QPointer>
#include <QReadLocker>

#include <config.h>

class PartitionTreeWidgetItem : public QTreeWidgetItem
{
	Q_DISABLE_COPY(PartitionTreeWidgetItem)

	public:
		PartitionTreeWidgetItem(const Partition* p) : QTreeWidgetItem(), m_Partition(p) {}
		const Partition* partition() const { return m_Partition; }

	private:
		const Partition* m_Partition;
};

/** Creates a new PartitionManagerWidget instance.
	@param parent the parent widget
	@param coll an action collection (may be NULL and set later)
*/
PartitionManagerWidget::PartitionManagerWidget(QWidget* parent) :
	QWidget(parent),
	Ui::PartitionManagerWidgetBase(),
	m_OperationStack(NULL),
	m_SelectedDevice(NULL),
	m_ClipboardPartition(NULL)
{
	setupUi(this);

	treePartitions().header()->setStretchLastSection(false);
	treePartitions().header()->setContextMenuPolicy(Qt::CustomContextMenu);
}

PartitionManagerWidget::~PartitionManagerWidget()
{
	saveConfig();
}

void PartitionManagerWidget::init(OperationStack* ostack)
{
	m_OperationStack = ostack;

	// TODO: shouldn't this also go to the main window class?
	FileSystemFactory::init();

	loadConfig();
	setupConnections();
}

void PartitionManagerWidget::loadConfig()
{
	QList<int> colWidths = Config::treePartitionColumnWidths();
	QList<int> colPositions = Config::treePartitionColumnPositions();
	QList<int> colVisible = Config::treePartitionColumnVisible();
	QHeaderView* header = treePartitions().header();

	for (int i = 0; i < treePartitions().columnCount(); i++)
	{
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

	for (int i = 0; i < treePartitions().columnCount(); i++)
	{
		colPositions.append(treePartitions().header()->visualIndex(i));
		colVisible.append(treePartitions().isColumnHidden(i) ? 0 : 1);
		colWidths.append(treePartitions().columnWidth(i));
	}

	Config::setTreePartitionColumnPositions(colPositions);
	Config::setTreePartitionColumnVisible(colVisible);
	Config::setTreePartitionColumnWidths(colWidths);

	Config::self()->writeConfig();
}

void PartitionManagerWidget::setupConnections()
{
	connect(treePartitions().header(), SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(onHeaderContextMenu(const QPoint&)));
}

void PartitionManagerWidget::clear()
{
	setSelectedDevice(NULL);
	setClipboardPartition(NULL);
	treePartitions().clear();
	partTableWidget().clear();
}

void PartitionManagerWidget::setSelectedPartition(const Partition* p)
{
	if (p == NULL)
	{
		treePartitions().setCurrentItem(NULL);
		emit selectedPartitionChanged(NULL);
		updatePartitions();
	}
	else
		partTableWidget().setActivePartition(p);
}

Partition* PartitionManagerWidget::selectedPartition()
{
	if (selectedDevice() == NULL || selectedDevice()->partitionTable() == NULL || partTableWidget().activeWidget() == NULL)
		return NULL;

	// The active partition we get from the part table widget is const; we need non-const.
	// So take the first sector and find the partition in the selected device's
	// partition table.
	const Partition* activePartition = partTableWidget().activeWidget()->partition();
	return selectedDevice()->partitionTable()->findPartitionBySector(activePartition->firstSector(), PartitionRole(PartitionRole::Any));
}

void PartitionManagerWidget::setSelectedDevice(const QString& device_node)
{
#if !defined(NO_THREADED_DEVICE_SCANNER)
	QReadLocker lockDevices(&operationStack().lock());
#endif

	foreach(Device* d, operationStack().previewDevices())
		if (d->deviceNode() == device_node)
		{
			setSelectedDevice(d);
			return;
		}

	setSelectedDevice(NULL);
}

void PartitionManagerWidget::setSelectedDevice(Device* d)
{
	m_SelectedDevice = d;
	setSelectedPartition(NULL);
}

static QTreeWidgetItem* createTreeWidgetItem(const Partition& p)
{
	QTreeWidgetItem* item = new PartitionTreeWidgetItem(&p);

	quint32 i = 0;
	item->setText(i++, p.deviceNode());

	item->setText(i, p.fileSystem().name());
	item->setIcon(i, createFileSystemColor(p.fileSystem().type(), 14));
	i++;

	item->setText(i, p.mountPoints().join(", "));
	if (p.isMounted())
		item->setIcon(i, SmallIcon("object-locked"));
	i++;

	item->setText(i++, p.fileSystem().label());
	item->setText(i++, p.fileSystem().uuid());
	item->setText(i++, Capacity(p).toString());
	item->setText(i++, Capacity(p, Capacity::Used).toString());
	item->setText(i++, Capacity(p, Capacity::Available).toString());

	item->setText(i++, KGlobal::locale()->formatNumber(p.firstSector(), 0));
	item->setText(i++, KGlobal::locale()->formatNumber(p.lastSector(), 0));
	item->setText(i++, KGlobal::locale()->formatNumber(p.length(), 0));

	item->setText(i++, PartitionTable::flagNames(p.activeFlags()).join(", "));

	item->setSizeHint(0, QSize(0, 32));

	return item;
}

void PartitionManagerWidget::updatePartitions()
{
	if (selectedDevice() == NULL)
		return;

	treePartitions().clear();
	partTableWidget().clear();

	partTableWidget().setPartitionTable(selectedDevice()->partitionTable());

	QTreeWidgetItem* deviceItem = new QTreeWidgetItem();
	deviceItem->setText(0, selectedDevice()->name());
	deviceItem->setIcon(0, DesktopIcon(selectedDevice()->iconName()));
	deviceItem->setSizeHint(0, QSize(0, 32));

	treePartitions().addTopLevelItem(deviceItem);

	if (selectedDevice()->partitionTable() != NULL)
	{
		foreach(const Partition* p, selectedDevice()->partitionTable()->children())
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
	}

	treePartitions().setFirstItemColumnSpanned(deviceItem, true);
	deviceItem->setExpanded(true);
	deviceItem->setFlags(Qt::ItemIsEnabled);

	partTableWidget().update();
}

void PartitionManagerWidget::on_m_TreePartitions_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem*)
{
	if (current)
	{
		const PartitionTreeWidgetItem* ptwItem = dynamic_cast<PartitionTreeWidgetItem*>(current);
		partTableWidget().setActivePartition(ptwItem ? ptwItem->partition() : NULL);
	}
	else
		partTableWidget().setActiveWidget(NULL);
}

void PartitionManagerWidget::on_m_TreePartitions_itemDoubleClicked(QTreeWidgetItem* item, int)
{
	if (item == treePartitions().topLevelItem(0))
	{
		if (selectedDevice() != NULL)
			emit deviceDoubleClicked(selectedDevice());
	}
	else
	{
		if (selectedPartition() != NULL)
			emit partitionDoubleClicked(selectedPartition());
	}
}

void PartitionManagerWidget::onHeaderContextMenu(const QPoint& p)
{
	showColumnsContextMenu(p, treePartitions());
}

void PartitionManagerWidget::on_m_PartTableWidget_itemSelectionChanged(PartWidget* item)
{
	if (item == NULL)
	{
		treePartitions().setCurrentItem(NULL);
		emit selectedPartitionChanged(NULL);
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

	emit selectedPartitionChanged(p);
}

void PartitionManagerWidget::on_m_PartTableWidget_customContextMenuRequested(const QPoint& pos)
{
	emit contextMenuRequested(partTableWidget().mapToGlobal(pos));
}

void PartitionManagerWidget::on_m_PartTableWidget_itemDoubleClicked()
{
	if (selectedPartition())
		emit partitionDoubleClicked(selectedPartition());
}

void PartitionManagerWidget::on_m_TreePartitions_customContextMenuRequested(const QPoint& pos)
{
	emit contextMenuRequested(treePartitions().viewport()->mapToGlobal(pos));
}

void PartitionManagerWidget::onPropertiesPartition()
{
	if (selectedPartition())
	{
		Partition& p = *selectedPartition();

		Q_ASSERT(selectedDevice());

		QPointer<PartPropsDialog> dlg = new PartPropsDialog(this, *selectedDevice(), p);

		if (dlg->exec() == KDialog::Accepted)
		{
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

	updatePartitions();
}

void PartitionManagerWidget::onEditMountPoint()
{
	Partition* p = selectedPartition();

	Q_ASSERT(p);

	if (p == NULL)
		return;

	QPointer<EditMountPointDialog> dlg = new EditMountPointDialog(this, *p);

	if (dlg->exec() == KDialog::Accepted)
		updatePartitions();

	delete dlg;
}

static bool checkTooManyPartitions(QWidget* parent, const Device& d, const Partition& p)
{
	Q_ASSERT(d.partitionTable());

	if (p.roles().has(PartitionRole::Unallocated) && d.partitionTable()->numPrimaries() >= d.partitionTable()->maxPrimaries() && !p.roles().has(PartitionRole::Logical))
	{
		KMessageBox::sorry(parent, i18ncp("@info",
			"<para>There is already one primary partition on this device. This is the maximum number its partition table type can handle.</para>"
			"<para>You cannot create, paste or restore a primary partition on it before you delete an existing one.</para>",
		   "<para>There are already %1 primary partitions on this device. This is the maximum number its partition table type can handle.</para>"
			"<para>You cannot create, paste or restore a primary partition on it before you delete an existing one.</para>",
			d.partitionTable()->numPrimaries()), i18nc("@title:window", "Too Many Primary Partitions."));
		return true;
	}

	return false;
}

void PartitionManagerWidget::onNewPartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	Q_ASSERT(selectedDevice()->partitionTable());

	if (selectedDevice()->partitionTable() == NULL)
	{
		kWarning() << "partition table on selected device is null";
		return;
	}

	if (checkTooManyPartitions(this, *selectedDevice(), *selectedPartition()))
		return;

	Partition* newPartition = NewOperation::createNew(*selectedPartition());

	QPointer<NewDialog> dlg = new NewDialog(this, *selectedDevice(), *newPartition, selectedDevice()->partitionTable()->childRoles(*selectedPartition()));
	if (dlg->exec() == KDialog::Accepted)
		operationStack().push(new NewOperation(*selectedDevice(), newPartition));
	else
		delete newPartition;

	delete dlg;
}

void PartitionManagerWidget::onDeletePartition(bool shred)
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

		if (selectedPartition()->number() > 0 && selectedPartition()->parent()->highestMountedChild() > selectedPartition()->number())
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
				KGuiItem(i18nc("@action:button", "Delete It")),
				KStandardGuiItem::cancel(), "reallyDeleteClipboardPartition") == KMessageBox::Cancel)
			return;

		setClipboardPartition(NULL);
	}

	operationStack().push(new DeleteOperation(*selectedDevice(), selectedPartition(), shred));
}

void PartitionManagerWidget::onShredPartition()
{
	onDeletePartition(true);
}

void PartitionManagerWidget::onResizePartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	Q_ASSERT(selectedDevice()->partitionTable());

	if (selectedDevice()->partitionTable() == NULL)
	{
		kWarning() << "partition table on selected device is null";
		return;
	}

	const qint64 freeBefore = selectedDevice()->partitionTable()->freeSectorsBefore(*selectedPartition());
	const qint64 freeAfter = selectedDevice()->partitionTable()->freeSectorsAfter(*selectedPartition());

	Partition resizedPartition(*selectedPartition());
	QPointer<ResizeDialog> dlg = new ResizeDialog(this, *selectedDevice(), resizedPartition, selectedPartition()->firstSector() - freeBefore, freeAfter + selectedPartition()->lastSector());

	if (dlg->exec() == KDialog::Accepted && dlg->isModified())
	{
		if (resizedPartition.firstSector() == selectedPartition()->firstSector() && resizedPartition.lastSector() == selectedPartition()->lastSector())
			Log(Log::information) << i18nc("@info/plain", "Partition <filename>%1</filename> has the same position and size after resize/move. Ignoring operation.", selectedPartition()->deviceNode());
		else
			operationStack().push(new ResizeOperation(*selectedDevice(), *selectedPartition(), resizedPartition.firstSector(), resizedPartition.lastSector()));
	}

	delete dlg;
}

void PartitionManagerWidget::onCopyPartition()
{
	Q_ASSERT(selectedPartition());

	if (selectedPartition() == NULL)
	{
		kWarning() << "selected partition: " << selectedPartition();
		return;
	}

	setClipboardPartition(selectedPartition());
	Log() << i18nc("@info/plain", "Partition <filename>%1</filename> has been copied to the clipboard.", selectedPartition()->deviceNode());
}

void PartitionManagerWidget::onPastePartition()
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
		operationStack().push(new CopyOperation(*selectedDevice(), copiedPartition, *dSource, clipboardPartition()));
	else
		delete copiedPartition;
}

bool PartitionManagerWidget::showInsertDialog(Partition& insertedPartition, qint64 sourceLength)
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
	// can PartitionTable::alignPartition() work correctly.
	selectedPartition()->parent()->reparent(insertedPartition);

	if (!overwrite)
	{
		QPointer<InsertDialog> dlg = new InsertDialog(this, *selectedDevice(), insertedPartition, *selectedPartition());

		int result = dlg->exec();
		delete dlg;

		if (result != KDialog::Accepted)
			return false;
	}

	if (insertedPartition.length() < sourceLength)
	{
		if (overwrite)
			KMessageBox::error(this, i18nc("@info",
				"<para>The selected partition is not large enough to hold the source partition or the backup file.</para>"
				"<para>Pick another target or resize this partition so it is as large as the source.</para>"), i18nc("@title:window", "Target Not Large Enough"));
		else
			KMessageBox::sorry(this, i18nc("@info",
				"<para>It is not possible to create the target partition large enough to hold the source.</para>"
				"<para>This may happen if not all partitions on a device are correctly aligned "
				"or when copying a primary partition into an extended partition.</para>"),
				i18nc("@title:window", "Cannot Create Target Partition."));
		return false;
	}

	return true;
}

void PartitionManagerWidget::onCheckPartition()
{
	Q_ASSERT(selectedDevice());
	Q_ASSERT(selectedPartition());

	if (selectedDevice() == NULL || selectedPartition() == NULL)
	{
		kWarning() << "selected device: " << selectedDevice() << ", selected partition: " << selectedPartition();
		return;
	}

	operationStack().push(new CheckOperation(*selectedDevice(), *selectedPartition()));
}

void PartitionManagerWidget::onBackupPartition()
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

	if (!QFile::exists(fileName) || KMessageBox::warningContinueCancel(this, i18nc("@info", "Do you want to overwrite the existing file <filename>%1</filename>?", fileName), i18nc("@title:window", "Overwrite Existing File?"), KGuiItem(i18nc("@action:button", "Overwrite File")), KStandardGuiItem::cancel()) == KMessageBox::Continue)
		operationStack().push(new BackupOperation(*selectedDevice(), *selectedPartition(), fileName));
}

void PartitionManagerWidget::onRestorePartition()
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
			operationStack().push(new RestoreOperation(*selectedDevice(), restorePartition, fileName));
		else
			delete restorePartition;
	}
}
