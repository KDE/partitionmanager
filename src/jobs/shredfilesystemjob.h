/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(SHREDFILESYSTEMJOB__H)

#define SHREDFILESYSTEMJOB__H

#include "jobs/job.h"

#include <QString>

class Partition;
class Device;
class Report;

/** Securely delete and shred a FileSystem.

	Shreds (overwrites with random data) a FileSystem on given Partition and Device.

	@author Volker Lanz <vl@fidra.de>
*/
class ShredFileSystemJob : public Job
{
	public:
		ShredFileSystemJob(Device& d, Partition& p);

	public:
		virtual bool run(Report& parent);
		virtual qint32 numSteps() const;
		virtual QString description() const;

	protected:
		Partition& partition() { return m_Partition; }
		const Partition& partition() const { return m_Partition; }

		Device& device() { return m_Device; }
		const Device& device() const { return m_Device; }

	private:
		Device& m_Device;
		Partition& m_Partition;
};

#endif
