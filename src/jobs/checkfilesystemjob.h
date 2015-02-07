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

#if !defined(CHECKFILESYSTEMJOB__H)

#define CHECKFILESYSTEMJOB__H

#include "jobs/job.h"

class Partition;
class Report;

class QString;

/** Check a FileSystem.
	@author Volker Lanz <vl@fidra.de>
*/
class CheckFileSystemJob : public Job
{
	public:
		CheckFileSystemJob(Partition& p);

	public:
		virtual bool run(Report& parent);
		virtual QString description() const;

	protected:
		Partition& partition() { return m_Partition; }
		const Partition& partition() const { return m_Partition; }

	private:
		Partition& m_Partition;
};

#endif
