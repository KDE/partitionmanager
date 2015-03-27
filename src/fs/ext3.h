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

#if !defined(EXT3__H)

#define EXT3__H

#include "util/libpartitionmanagerexport.h"

#include "fs/ext2.h"

#include <qglobal.h>

class Report;

class QString;

namespace FS
{
	/** An ext3 file system.

		Basically the same as ext2.

		@author Volker Lanz <vl@fidra.de>
	 */
	class LIBKPMCORE_EXPORT ext3 : public ext2
	{
		public:
			ext3(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

		public:
			static void init() {}
			virtual bool create(Report& report, const QString& deviceNode) const;
			virtual qint64 maxCapacity() const;
	};
}

#endif
