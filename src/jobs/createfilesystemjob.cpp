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

#include "jobs/createfilesystemjob.h"

#include "core/partition.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <klocale.h>

/** Creates a new CreateFileSystemJob
	@param p the Partition the FileSystem to create is on
*/
CreateFileSystemJob::CreateFileSystemJob(Partition& p) :
	Job(),
	m_Partition(p)
{
}

bool CreateFileSystemJob::run(Report& parent)
{
	bool rval = false;

	Report* report = jobStarted(parent);
	
	if (partition().fileSystem().supportCreate() == FileSystem::cmdSupportFileSystem)
		rval = partition().fileSystem().create(*report, partition().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString CreateFileSystemJob::description() const
{
	return i18nc("@info/plain", "Create file system %1 on partition <filename>%2</filename>", partition().fileSystem().name(), partition().deviceNode());
}
