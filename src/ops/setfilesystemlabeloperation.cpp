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

#include "ops/setfilesystemlabeloperation.h"

#include "core/partition.h"

#include "jobs/setfilesystemlabeljob.h"

#include "fs/filesystem.h"

#include <QString>

#include <kdebug.h>
#include <klocale.h>

/** Creates a new SetFileSystemLabelOperation.
	@param p the Partition with the FileSystem to set the label for
	@param newlabel the new label
*/
SetFileSystemLabelOperation::SetFileSystemLabelOperation(Partition& p, const QString& newlabel) :
	Operation(),
	m_LabeledPartition(p),
	m_OldLabel(labeledPartition().fileSystem().label()),
	m_NewLabel(newlabel),
	m_LabelJob(new SetFileSystemLabelJob(labeledPartition(), newLabel()))
{
	addJob(labelJob());
}

void SetFileSystemLabelOperation::preview()
{
	labeledPartition().fileSystem().setLabel(newLabel());
}

void SetFileSystemLabelOperation::undo()
{
	labeledPartition().fileSystem().setLabel(oldLabel());
}

QString SetFileSystemLabelOperation::description() const
{
	if (oldLabel().isEmpty())
		return QString(i18nc("@info/plain", "Set label for partition <filename>%1</filename> to \"%2\"", labeledPartition().deviceNode(), newLabel()));
	
	return QString(i18nc("@info/plain", "Set label for partition <filename>%1</filename> from \"%2\" to \"%3\"", labeledPartition().deviceNode(), oldLabel(), newLabel()));
}
