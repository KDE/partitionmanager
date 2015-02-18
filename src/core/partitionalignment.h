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

#if !defined(PARTITIONALIGNMENT__H)

#define PARTITIONALIGNMENT__H

#include "qglobal.h"

#include "util/libpartitionmanagerexport.h"

class Device;
class Partition;

class LIBKPMCORE_EXPORT PartitionAlignment
{
	private:
		PartitionAlignment();

	public:
		static bool isAligned(const Device& d, const Partition& p, bool quiet = false);
		static bool isAligned(const Device& d, const Partition& p, qint64 newFirst, qint64 newLast, bool quiet);

		static qint64 alignedFirstSector(const Device& d, const Partition& p, qint64 s, qint64 min_first, qint64 max_first, qint64 min_length, qint64 max_length);

		static qint64 alignedLastSector(const Device& d, const Partition& p, qint64 s, qint64 min_last, qint64 max_last, qint64 min_length, qint64 max_length, qint64 original_length = -1, bool original_aligned = false);

		static qint64 sectorAlignment(const Device& d);

		static qint64 firstDelta(const Device& d, const Partition& p, qint64 s);

		static qint64 lastDelta(const Device& d, const Partition& p, qint64 s);

		static bool isLengthAligned(const Device& d, const Partition& p);
};

#endif
