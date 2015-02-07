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

#if !defined(SETFILESYSTEMLABELJOB__H)

#define SETFILESYSTEMLABELJOB__H

#include "jobs/job.h"

#include <QString>

class Partition;
class Report;
class OperationStack;

/** Set a FileSystem label.
	@author Volker Lanz <vl@fidra.de>
*/
class SetFileSystemLabelJob : public Job
{
	friend class OperationStack;
	
	public:
		SetFileSystemLabelJob(Partition& p, const QString& newlabel);

	public:
		virtual bool run(Report& parent);
		virtual QString description() const;

	protected:
		Partition& partition() { return m_Partition; }
		const Partition& partition() const { return m_Partition; }

		const QString& label() const { return m_Label; }
		void setLabel(const QString& l) { m_Label = l; }

	private:
		Partition& m_Partition;
		QString m_Label;
};

#endif
