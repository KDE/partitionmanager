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

#if !defined(BACKUPFILESYSTEMJOB__H)

#define BACKUPFILESYSTEMJOB__H

#include "jobs/job.h"

#include <QString>

class Partition;
class Device;
class Report;

/** Back up a FileSystem.

	Backs up a FileSystem from a given Device and Partition to a file with the given filename.

	@author Volker Lanz <vl@fidra.de>
*/
class BackupFileSystemJob : public Job
{
	public:
		BackupFileSystemJob(Device& sourcedevice, Partition& sourcepartition, const QString& filename);

	public:
		virtual bool run(Report& parent);
		virtual qint32 numSteps() const;
		virtual QString description() const;
		
	protected:
		Partition& sourcePartition() { return m_SourcePartition; }
		const Partition& sourcePartition() const { return m_SourcePartition; }

		Device& sourceDevice() { return m_SourceDevice; }
		const Device& sourceDevice() const { return m_SourceDevice; }

		const QString& fileName() const { return m_FileName; }

	private:
		Device& m_SourceDevice;
		Partition& m_SourcePartition;
		QString m_FileName;
};

#endif
