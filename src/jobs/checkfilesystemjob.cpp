/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#include "jobs/checkfilesystemjob.h"

#include "core/partition.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CheckFileSystemJob
	@param p the Partition whose FileSystem is to be checked
*/
CheckFileSystemJob::CheckFileSystemJob(Partition& p) :
	Job(),
	m_Partition(p)
{
}

bool CheckFileSystemJob::run(Report& parent)
{
	Report* report = jobStarted(parent);

	// if we cannot check, assume everything is fine
	bool rval = true;

	if (partition().fileSystem().supportCheck() == FileSystem::cmdSupportFileSystem)
		rval = partition().fileSystem().check(*report, partition().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString CheckFileSystemJob::description() const
{
	return xi18nc("@info/plain", "Check file system on partition <filename>%1</filename>", partition().deviceNode());
}
