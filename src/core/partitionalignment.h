/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#if !defined(PARTITIONALIGNMENT__H)

#define PARTITIONALIGNMENT__H

#include "qglobal.h"

#include "util/libpartitionmanagerexport.h"

class Device;
class Partition;

class LIBPARTITIONMANAGERPRIVATE_EXPORT PartitionAlignment
{
	public:
		PartitionAlignment(const Device& d, const Partition& p, const Partition* op = NULL);

	public:
		bool alignFirstSector(const Device& d, Partition& p);
		bool alignLastSector(const Device& d, Partition& p);
		bool checkAlignConstraints(const Device& d, Partition& p);
		bool alignChildren(const Device& d, Partition& p);
		bool canAlignToSector(const Device& d, const Partition& p, qint64 s) const;

		static qint64 sectorAlignment(const Device& d);
		static bool isAligned(const Device& d, const Partition& p);
		static bool alignPartition(const Device& d, Partition& p, const Partition* originalPartition = NULL);

		qint64 firstDelta() const { return m_FirstDelta; }
		qint64 lastDelta() const { return m_LastDelta; }
		bool isLengthAligned() const { return m_LengthAligned; }
		const Partition* originalPartition() const { return m_OriginalPartition; }
		qint64 originalLength() const { return m_OriginalLength; }

	private:
		qint64 m_FirstDelta;
		qint64 m_LastDelta;
		bool m_LengthAligned;
		const Partition* m_OriginalPartition;
		qint64 m_OriginalLength;
};

#endif
