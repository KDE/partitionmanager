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

#include "core/operationstack.h"
#include "core/device.h"
#include "core/partition.h"
#include "core/partitiontable.h"

#include "ops/operation.h"
#include "ops/deleteoperation.h"
#include "ops/newoperation.h"
#include "ops/resizeoperation.h"
#include "ops/copyoperation.h"
#include "ops/restoreoperation.h"
#include "ops/createfilesystemoperation.h"
#include "ops/setpartflagsoperation.h"
#include "ops/setfilesystemlabeloperation.h"

#include "jobs/setfilesystemlabeljob.h"

#include "fs/filesystemfactory.h"

#include "util/globallog.h"

#include <kdebug.h>
#include <klocale.h>

#include <QReadLocker>
#include <QWriteLocker>

/** Constructs a new OperationStack */
OperationStack::OperationStack(QObject* parent) :
	QObject(parent),
	m_Operations(),
	m_PreviewDevices(),
	m_Lock(QReadWriteLock::Recursive)
{
}

/** Destructs an OperationStack, cleaning up Operations and Devices */
OperationStack::~OperationStack()
{
	clearOperations();
	clearDevices();
}

/** Tries to merge an existing NewOperation with a new Operation pushed on the OperationStack

	There are several cases what might need to be done:

	<ol>
	<!-- 1 -->
	<li>An existing operation created a Partition that is now being deleted: In this case, just remove
		the corresponding NewOperation from the OperationStack.</li>
	<!-- 2 -->
	<li>An existing Operation created a Partition that is now being moved or resized. In this case,
		remove the original NewOperation and create a new NewOperation with updated start and end
		sectors. This new NewOperation is appended to the OperationStack.</li>
	<!-- 3 -->
	<li>An existing NewOperation created a Partition that is now being copied. We're not copying
		but instead creating another new Partition in its place.</li>
	<!-- 4 -->
	<li>The label for a new Partition's FileSystem is modified: Modify in NewOperation and forget it.</li>
	<!-- 5 -->
	<li>File system is changed for a new Partition: Modify in NewOperation and forget it.</li>
	</ol>

	@param currentOp the Operation already on the stack to try to merge with
	@param pushedOp the newly pushed Operation
	@return true if the OperationStack has been modified in a way that requires merging to stop
*/
bool OperationStack::mergeNewOperation(Operation*& currentOp, Operation*& pushedOp)
{
	NewOperation* newOp = dynamic_cast<NewOperation*>(currentOp);

	if (newOp == NULL)
		return false;

	DeleteOperation* pushedDeleteOp = dynamic_cast<DeleteOperation*>(pushedOp);
	ResizeOperation* pushedResizeOp = dynamic_cast<ResizeOperation*>(pushedOp);
	CopyOperation* pushedCopyOp = dynamic_cast<CopyOperation*>(pushedOp);
	SetFileSystemLabelOperation* pushedLabelOp = dynamic_cast<SetFileSystemLabelOperation*>(pushedOp);
	CreateFileSystemOperation* pushedCreateFileSystemOp = dynamic_cast<CreateFileSystemOperation*>(pushedOp);

	// -- 1 --
	if (pushedDeleteOp && &newOp->newPartition() == &pushedDeleteOp->deletedPartition())
	{
		Log() << i18nc("@info/plain", "Deleting a partition just created: Undoing the operation to create the partition.");

		delete pushedOp;
		pushedOp = NULL;

		newOp->undo();
		delete operations().takeAt(operations().indexOf(newOp));

		return true;
	}

	// -- 2 --
	if (pushedResizeOp && &newOp->newPartition() == &pushedResizeOp->partition())
	{
		Log() << i18nc("@info/plain", "Resizing a partition just created: Updating start and end in existing operation.");

		Partition* newPartition = new Partition(newOp->newPartition());
		newPartition->setFirstSector(pushedResizeOp->newFirstSector());
		newPartition->fileSystem().setFirstSector(pushedResizeOp->newFirstSector());
		newPartition->setLastSector(pushedResizeOp->newLastSector());
		newPartition->fileSystem().setLastSector(pushedResizeOp->newLastSector());

		NewOperation* revisedNewOp = new NewOperation(newOp->targetDevice(), newPartition);
		delete pushedOp;
		pushedOp = revisedNewOp;

		newOp->undo();
		delete operations().takeAt(operations().indexOf(newOp));

		return true;
	}

	// -- 3 --
	if (pushedCopyOp && &newOp->newPartition() == &pushedCopyOp->sourcePartition())
	{
		Log() << i18nc("@info/plain", "Copying a new partition: Creating a new partition instead.");

		Partition* newPartition = new Partition(newOp->newPartition());
		newPartition->setFirstSector(pushedCopyOp->copiedPartition().firstSector());
		newPartition->fileSystem().setFirstSector(pushedCopyOp->copiedPartition().fileSystem().firstSector());
		newPartition->setLastSector(pushedCopyOp->copiedPartition().lastSector());
		newPartition->fileSystem().setLastSector(pushedCopyOp->copiedPartition().fileSystem().lastSector());

		NewOperation* revisedNewOp = new NewOperation(pushedCopyOp->targetDevice(), newPartition);
		delete pushedOp;
		pushedOp = revisedNewOp;

		return true;
	}

	// -- 4 --
	if (pushedLabelOp && &newOp->newPartition() == &pushedLabelOp->labeledPartition())
	{
		Log() << i18nc("@info/plain", "Changing label for a new partition: No new operation required.");

		newOp->setLabelJob()->setLabel(pushedLabelOp->newLabel());
		newOp->newPartition().fileSystem().setLabel(pushedLabelOp->newLabel());

		delete pushedOp;
		pushedOp = NULL;

		return true;
	}

	// -- 5 --
	if (pushedCreateFileSystemOp && &newOp->newPartition() == &pushedCreateFileSystemOp->partition())
	{
		Log() << i18nc("@info/plain", "Changing file system for a new partition: No new operation required.");

		FileSystem* oldFs = &newOp->newPartition().fileSystem();

		newOp->newPartition().setFileSystem(FileSystemFactory::cloneWithNewType(pushedCreateFileSystemOp->newFileSystem()->type(), *oldFs));

		delete oldFs;
		oldFs = NULL;

		delete pushedOp;
		pushedOp = NULL;

		return true;
	}

	return false;
}

/**	Tries to merge an existing CopyOperation with a new Operation pushed on the OperationStack.

	These are the cases to consider:

	<ol>
	<!-- 1 -->
	<li>An existing CopyOperation created a Partition that is now being deleted. Remove the
		CopyOperation, and, if the CopyOperation was an overwrite, carry on with the delete. Else
		also remove the DeleteOperation.</li>

	<!-- 2 -->
	<li>An existing CopyOperation created a Partition that is now being copied. We're not copying
		the target of this existing CopyOperation, but its source instead. If this merge is not done,
		copied partitions will have misleading labels ("copy of sdXY" instead of "copy of copy of
		sdXY" for a second-generation copy) but the Operation itself will still work.</li>
	</ol>

	@param currentOp the Operation already on the stack to try to merge with
	@param pushedOp the newly pushed Operation
	@return true if the OperationStack has been modified in a way that requires merging to stop
*/
bool OperationStack::mergeCopyOperation(Operation*& currentOp, Operation*& pushedOp)
{
	CopyOperation* copyOp = dynamic_cast<CopyOperation*>(currentOp);

	if (copyOp == NULL)
		return false;

	DeleteOperation* pushedDeleteOp = dynamic_cast<DeleteOperation*>(pushedOp);
	CopyOperation* pushedCopyOp = dynamic_cast<CopyOperation*>(pushedOp);

	// -- 1 --
	if (pushedDeleteOp && &copyOp->copiedPartition() == &pushedDeleteOp->deletedPartition())
	{
		// If the copy operation didn't overwrite, but create a new partition, just remove the
		// copy operation, forget the delete and be done.
		if (copyOp->overwrittenPartition() == NULL)
		{
			Log() << i18nc("@info/plain", "Deleting a partition just copied: Removing the copy.");

			delete pushedOp;
			pushedOp = NULL;
		}
		else
		{
			Log() << i18nc("@info/plain", "Deleting a partition just copied over an existing partition: Removing the copy and deleting the existing partition.");

			pushedDeleteOp->setDeletedPartition(copyOp->overwrittenPartition());
		}

		copyOp->undo();
		delete operations().takeAt(operations().indexOf(copyOp));

		return true;
	}

	// -- 2 --
	if (pushedCopyOp && &copyOp->copiedPartition() == &pushedCopyOp->sourcePartition())
	{
		Log() << i18nc("@info/plain", "Copying a partition that is itself a copy: Copying the original source partition instead.");

		pushedCopyOp->setSourcePartition(&copyOp->sourcePartition());
	}

	return false;
}

/** Tries to merge an existing RestoreOperation with a new Operation pushed on the OperationStack.

	If an existing RestoreOperation created a Partition that is now being deleted, remove the
	RestoreOperation, and, if the RestoreOperation was an overwrite, carry on with the delete. Else
	also remove the DeleteOperation.

	@param currentOp the Operation already on the stack to try to merge with
	@param pushedOp the newly pushed Operation
	@return true if the OperationStack has been modified in a way that requires merging to stop
*/
bool OperationStack::mergeRestoreOperation(Operation*& currentOp, Operation*& pushedOp)
{
	RestoreOperation* restoreOp = dynamic_cast<RestoreOperation*>(currentOp);

	if (restoreOp == NULL)
		return false;

	DeleteOperation* pushedDeleteOp = dynamic_cast<DeleteOperation*>(pushedOp);

	if (pushedDeleteOp && &restoreOp->restorePartition() == &pushedDeleteOp->deletedPartition())
	{
		if (restoreOp->overwrittenPartition() == NULL)
		{
			Log() << i18nc("@info/plain", "Deleting a partition just restored: Removing the restore operation.");

			delete pushedOp;
			pushedOp = NULL;
		}
		else
		{
			Log() << i18nc("@info/plain", "Deleting a partition just restored to an existing partition: Removing the restore operation and deleting the existing partition.");

			pushedDeleteOp->setDeletedPartition(restoreOp->overwrittenPartition());
		}

		restoreOp->undo();
		delete operations().takeAt(operations().indexOf(restoreOp));

		return true;
	}

	return false;
}

/** Tries to merge an existing SetPartFlagsOperation with a new Operation pushed on the OperationStack.

	If the Partition flags for an existing Partition are modified look if there is an existing
	Operation for the same Partition and modify that one.

	@param currentOp the Operation already on the stack to try to merge with
	@param pushedOp the newly pushed Operation
	@return true if the OperationStack has been modified in a way that requires merging to stop
*/
bool OperationStack::mergePartFlagsOperation(Operation*& currentOp, Operation*& pushedOp)
{
	SetPartFlagsOperation* partFlagsOp = dynamic_cast<SetPartFlagsOperation*>(currentOp);

	if (partFlagsOp == NULL)
		return false;

	SetPartFlagsOperation* pushedFlagsOp = dynamic_cast<SetPartFlagsOperation*>(pushedOp);

	if (pushedFlagsOp && &partFlagsOp->flagPartition() == &pushedFlagsOp->flagPartition())
	{
		Log() << i18nc("@info/plain", "Changing flags again for the same partition: Removing old operation.");

		pushedFlagsOp->setOldFlags(partFlagsOp->oldFlags());
		partFlagsOp->undo();
		delete operations().takeAt(operations().indexOf(partFlagsOp));

		return true;
	}

	return false;
}

/** Tries to merge an existing SetFileSystemLabelOperation with a new Operation pushed on the OperationStack.

	If a FileSystem label for an existing Partition is modified look if there is an existing
	SetFileSystemLabelOperation for the same Partition.

	@param currentOp the Operation already on the stack to try to merge with
	@param pushedOp the newly pushed Operation
	@return true if the OperationStack has been modified in a way that requires merging to stop
*/
bool OperationStack::mergePartLabelOperation(Operation*& currentOp, Operation*& pushedOp)
{
	SetFileSystemLabelOperation* partLabelOp = dynamic_cast<SetFileSystemLabelOperation*>(currentOp);

	if (partLabelOp == NULL)
		return false;

	SetFileSystemLabelOperation* pushedLabelOp = dynamic_cast<SetFileSystemLabelOperation*>(pushedOp);

	if (pushedLabelOp && &partLabelOp->labeledPartition() == &pushedLabelOp->labeledPartition())
	{
		Log() << i18nc("@info/plain", "Changing label again for the same partition: Removing old operation.");

		pushedLabelOp->setOldLabel(partLabelOp->oldLabel());
		partLabelOp->undo();
		delete operations().takeAt(operations().indexOf(partLabelOp));

		return true;
	}

	return false;
}

/** Pushes a new Operation on the OperationStack.

	This method will call all methods that try to merge the new Operation with the
	existing ones. It is not uncommon that any of these will delete the pushed
	Operation. Callers <b>must not rely</b> on the pushed Operation to exist after
	calling OperationStack::push().

	@param o Pointer to the Operation. Must not be NULL.
*/
void OperationStack::push(Operation* o)
{
	Q_ASSERT(o);

	foreach (Operation* currentOp, operations())
	{
		if (mergeNewOperation(currentOp, o))
			break;

		if (mergeCopyOperation(currentOp, o))
			break;

		if (mergeRestoreOperation(currentOp, o))
			break;

		if (mergePartFlagsOperation(currentOp, o))
			break;

		if (mergePartLabelOperation(currentOp, o))
			break;
	}

	if (o != NULL)
	{
		Log() << i18nc("@info/plain", "Add operation: %1", o->description());
		operations().append(o);
		o->preview();
		o->setStatus(Operation::StatusPending);
	}

	// emit operationsChanged even if o is NULL because it has been merged: merging might
	// have led to an existing operation changing.
	emit operationsChanged();
}

/** Removes the topmost Operation from the OperationStack, calls Operation::undo() on it and deletes it. */
void OperationStack::pop()
{
	Operation* o = operations().takeLast();
	o->undo();
	delete o;
	emit operationsChanged();
}

/** Removes all Operations from the OperationStack, calling Operation::undo() on them and deleting them. */
void OperationStack::clearOperations()
{
	while (!operations().isEmpty())
	{
		Operation* o = operations().takeLast();
		if (o->status() == Operation::StatusPending)
			o->undo();
		delete o;
	}

	emit operationsChanged();
}

/** Clears the list of Devices. */
void OperationStack::clearDevices()
{
#if !defined(NO_THREADED_DEVICE_SCANNER)
	QWriteLocker lockDevices(&lock());
#endif

	qDeleteAll(previewDevices());
	previewDevices().clear();
	emit devicesChanged();
}

/** Finds a Device a Partition is on.
	@param p pointer to the Partition to find a Device for
	@return the Device or NULL if none could be found
*/
Device* OperationStack::findDeviceForPartition(const Partition* p)
{
#if !defined(NO_THREADED_DEVICE_SCANNER)
	QReadLocker lockDevices(&lock());
#endif

	foreach (Device* d, previewDevices())
	{
		if (d->partitionTable() == NULL)
			continue;

		foreach(const Partition* part, d->partitionTable()->children())
		{
			if (part == p)
				return d;

			foreach (const Partition* child, part->children())
				if (child == p)
					return d;
		}
	}

	return NULL;
}

/** Adds a Device to the OperationStack
	@param d pointer to the Device to add. Must not be NULL.
*/
void OperationStack::addDevice(Device* d)
{
	Q_ASSERT(d);

#if !defined(NO_THREADED_DEVICE_SCANNER)
	QWriteLocker lockDevices(&lock());
#endif

	previewDevices().append(d);
	emit devicesChanged();
}

static bool deviceLessThan(const Device* d1, const Device* d2)
{
	return d1->deviceNode() <= d2->deviceNode();
}

void OperationStack::sortDevices()
{
#if !defined(NO_THREADED_DEVICE_SCANNER)
	QWriteLocker lockDevices(&lock());
#endif

	qSort(previewDevices().begin(), previewDevices().end(), deviceLessThan);

	emit devicesChanged();
}
