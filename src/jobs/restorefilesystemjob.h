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

#if !defined(RESTOREFILESYSTEMJOB__H)

#define RESTOREFILESYSTEMJOB__H

#include "jobs/job.h"

#include <QString>

class Partition;
class Device;
class Report;

/** Restore a FileSystem.

	Restores a FileSystem from a file to a given Partition on a given Device.

	@author Volker Lanz <vl@fidra.de>
*/
class RestoreFileSystemJob : public Job
{
	public:
		RestoreFileSystemJob(Device& targetdevice, Partition& targetpartition, const QString& filename);

	public:
		virtual bool run(Report& parent);
		virtual qint32 numSteps() const;
		virtual QString description() const;

	protected:
		Partition& targetPartition() { return m_TargetPartition; }
		const Partition& targetPartition() const { return m_TargetPartition; }

		Device& targetDevice() { return m_TargetDevice; }
		const Device& targetDevice() const { return m_TargetDevice; }

		const QString& fileName() const { return m_FileName; }

	private:
		Device& m_TargetDevice;
		Partition& m_TargetPartition;
		QString m_FileName;
};

#endif
