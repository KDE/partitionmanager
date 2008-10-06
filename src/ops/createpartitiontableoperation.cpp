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

#include "ops/createpartitiontableoperation.h"

#include "core/device.h"
#include "core/partitiontable.h"
#include "core/partition.h"

#include "jobs/createpartitiontablejob.h"

#include <QString>

#include <kdebug.h>
#include <klocale.h>

/** Creates a new CreatePartitionTableOperation.
	@param d the Device to create the new PartitionTable on
*/
CreatePartitionTableOperation::CreatePartitionTableOperation(Device& d) :
	Operation(),
	m_TargetDevice(d),
	m_OldPartitionTable(&targetDevice().partitionTable()),
	m_PartitionTable(new PartitionTable()),
	m_CreatePartitionTableJob(new CreatePartitionTableJob(targetDevice()))
{
	addJob(createPartitionTableJob());
}

CreatePartitionTableOperation::~CreatePartitionTableOperation()
{
	if (status() == StatusPending)
		delete m_PartitionTable;
}

void CreatePartitionTableOperation::preview()
{
	targetDevice().setPartitionTable(&partitionTable());
	targetDevice().partitionTable().updateUnallocated(targetDevice());
}

void CreatePartitionTableOperation::undo()
{
	targetDevice().setPartitionTable(&oldPartitionTable());
	targetDevice().partitionTable().updateUnallocated(targetDevice());
}

bool CreatePartitionTableOperation::execute(Report& parent)
{
	targetDevice().setPartitionTable(&partitionTable());
	return Operation::execute(parent);
}

/** Can a new partition table be created on a device?
	@param device pointer to the device, can be NULL
	@return true if a new partition table can be created on @p device
*/
bool CreatePartitionTableOperation::canCreate(const Device* device)
{
	return device != NULL && !device->partitionTable().isChildMounted();
}

QString CreatePartitionTableOperation::description() const
{
	return QString(i18nc("@info/plain", "Create a new partition table on <filename>%1</filename>", targetDevice().deviceNode()));
}
