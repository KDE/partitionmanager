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

#include "core/partitionalignment.h"

#include "core/partition.h"
#include "core/partitiontable.h"
#include "core/device.h"

#include "fs/filesystem.h"

#include "util/globallog.h"

#include <klocale.h>
#include <kdebug.h>

#include <config.h>

PartitionAlignment::PartitionAlignment(const Device& d, const Partition& p, const Partition* op) :
	m_FirstDelta(0),
	m_LastDelta(0),
	m_LengthAligned(false),
	m_OriginalPartition(op),
	m_OriginalLength(0)
{
	if (d.partitionTable()->type() == PartitionTable::msdos)
	{
		if (p.roles().has(PartitionRole::Logical) && p.firstSector() == 2 * d.sectorsPerTrack())
		{
			m_FirstDelta = (p.firstSector() - (2 * d.sectorsPerTrack())) % sectorAlignment(d);
			m_LengthAligned = (p.length() + (2 * d.sectorsPerTrack())) % sectorAlignment(d) == 0;
		}
		else if (p.roles().has(PartitionRole::Logical) || p.firstSector() == d.sectorsPerTrack())
		{
			m_FirstDelta = (p.firstSector() - d.sectorsPerTrack()) % sectorAlignment(d);
			m_LengthAligned = (p.length() + d.sectorsPerTrack()) % sectorAlignment(d) == 0;
		}
		else
		{
			m_FirstDelta = p.firstSector() % sectorAlignment(d);
			m_LengthAligned = p.length() % sectorAlignment(d) == 0;
		}
	}
	else
	{
		m_FirstDelta = p.firstSector() % sectorAlignment(d);
		m_LengthAligned = p.length() % sectorAlignment(d) == 0;
	}

	m_LastDelta = (p.lastSector() + 1) % sectorAlignment(d);
}

/** Aligns the given Partition on the given Device to the PartitionTable's required alignment.

	Tries under all accounts to keep the Partition's length equal to the original length or
	to increase it, if that is not possible. Will print a warning message to GlobalLog if
	this is not possible.

	The parameter @p originalPartition is required for cases where a Partition has just been
	duplicated to resize or move it. This method needs to know the original because of course
	the original does not prevent aligning to any sector allocated by it.

	@see canAlignToSector(), isAligned()

	@param d the Device the Partition is on
	@param p the Partition to align
	@param originalPartition pointer to a Partition object @p p has just been copied from or NULL
	@return true if Partition is now properly aligned
*/
bool PartitionAlignment::alignPartition(const Device& d, Partition& p, const Partition* originalPartition)
{
	PartitionAlignment pa(d, p, originalPartition);

	pa.alignFirstSector(d, p);
	pa.alignLastSector(d, p);
	pa.checkAlignConstraints(d, p);
	pa.alignChildren(d, p);

	return pa.isAligned(d, p);
}


/** Checks if a given Partition on a given Device is properly aligned to the PartitionTable's
	alignment requirements.

	Will print warning messages to GlobalLog if the Partition's first sector is not aligned and
	another one if the last sector is not aligned.

	@see alignPartition(), canAlignToSector()

	@param d the Device the Partition is on
	@param p the Partition to check
	@return true if propertly aligned
*/
bool PartitionAlignment::isAligned(const Device& d, const Partition& p)
{
	// don't bother with unallocated space here.
	if (p.roles().has(PartitionRole::Unallocated))
		return true;

	qint64 delta = 0;

	if (d.partitionTable()->type() == PartitionTable::msdos)
	{
		// TODO: verify the following comment and code
		// There are some special cases for aligning partitions:
		// 1) If an extended partition starts at the beginning of the device (that would be sector 63
		// on modern drives, equivalent to sectorsPerTrack() in any case), the first logical partition
		// at the beginning of this extended partition starts at 2 * sectorsPerTrack().
		// 2) If a primary or extended starts at the beginning of a device, it starts at sectorsPerTrack().
		// 3) Any logical partition is always preceded by the extended partition table entry in the
		// sectorsPerTrack() before it, so it's always sectorsPerTrack() "late"
		if (p.roles().has(PartitionRole::Logical) && p.firstSector() == 2 * d.sectorsPerTrack())
			delta = (p.firstSector() - (2 * d.sectorsPerTrack())) % sectorAlignment(d);
		else if (p.roles().has(PartitionRole::Logical) || p.firstSector() == d.sectorsPerTrack())
			delta = (p.firstSector() - d.sectorsPerTrack()) % sectorAlignment(d);
		else
			delta = p.firstSector() % sectorAlignment(d);
	}
	else
		delta = p.firstSector() % sectorAlignment(d);

	bool rval = true;

	if (delta)
	{
		Log(Log::warning) << i18nc("@info/plain", "Partition <filename>%1</filename> is not properly aligned (first sector: %2, modulo: %3).", p.deviceNode(), p.firstSector(), delta);
		rval = false;
	}

	delta = (p.lastSector() + 1) % sectorAlignment(d);

	if (delta)
	{
		Log(Log::warning) << i18nc("@info/plain", "Partition <filename>%1</filename> is not properly aligned (last sector: %2, modulo: %3).", p.deviceNode(), p.lastSector(), delta);
		rval = false;
	}

	return rval;
}


/** @return the sector size to align the partition start and end to
*/
qint64 PartitionAlignment::sectorAlignment(const Device& d)
{
	return d.partitionTable()->type() == PartitionTable::msdos ? d.cylinderSize() : Config::sectorAlignment();
}

bool PartitionAlignment::alignFirstSector(const Device& d, Partition& p)
{
	bool rval = false;

	if (firstDelta())
	{
		/** @todo Don't assume we always want to align to the front.
			Always trying to align to the front solves the problem that a partition does
			get too small to take another one that's copied to it, but it introduces
			a new bug: The user might create a partition aligned at the end of a device,
			extended partition or at the start of the next one, but we align to the back
			and leave some space in between.
		*/
		// We always want to make the partition larger, not smaller. Making it smaller
		// might, in case it's a partition that another is being copied to, mean the partition
		// ends up too small. So try to move the start to the front first.
		qint64 alignedFirst = p.firstSector() - firstDelta();

		// If we're now before the first usable sector, just take the first usable sector. This
		// will happen if we're already below cylinder one and align to the front
		if (alignedFirst < d.partitionTable()->firstUsable())
			alignedFirst = d.partitionTable()->firstUsable();

		// Now if the cylinder boundary at the front is occupied...
		if (!canAlignToSector(d, p, alignedFirst))
		{
			// ... move to the cylinder towards the end of the device ...
			alignedFirst = p.firstSector() - firstDelta() + sectorAlignment(d);

			// ... and move the end of the partition towards the end, too, if that is possible.
			// By doing this, we still try to keep the length >= the original length. If the
			// last sector ends up not being on a cylinder boundary by doing so, the code
			// below will deal with that.
			qint64 numTooShort = sectorAlignment(d) - firstDelta();
			if (canAlignToSector(d, p, p.lastSector() + numTooShort))
			{
				p.setLastSector(p.lastSector() + numTooShort);
				p.fileSystem().setLastSector(p.fileSystem().lastSector() + numTooShort);
			}
		}

		rval = alignedFirst != p.firstSector();

		p.setFirstSector(alignedFirst);
		p.fileSystem().setFirstSector(alignedFirst);
	}

	return rval;
}

bool PartitionAlignment::alignLastSector(const Device& d, Partition& p)
{
	bool rval = false;

	if (lastDelta())
	{
		// Try to align to the back first...
		qint64 alignedLast = p.lastSector() + sectorAlignment(d) - lastDelta();

		// .. but if we can retain the partition length exactly by aligning to the front ...
		if (isLengthAligned() && p.length() - originalLength() == lastDelta())
			alignedLast -= sectorAlignment(d);
		// ... or if there's something there already, align to the front.
		else if (!canAlignToSector(d, p, alignedLast))
			alignedLast -= sectorAlignment(d);

		rval = alignedLast != p.lastSector();
		p.setLastSector(alignedLast);
		p.fileSystem().setLastSector(alignedLast);
	}

	return rval;
}

bool PartitionAlignment::checkAlignConstraints(const Device& d, Partition& p)
{
	bool rval = false;

	// Now, did we make the partition too big for its file system?
	while (p.length() > originalLength() && p.capacity() > p.fileSystem().maxCapacity() && canAlignToSector(d, p, p.lastSector() - sectorAlignment(d)))
	{
		rval = true;
		p.setLastSector(p.lastSector() - sectorAlignment(d));
		p.fileSystem().setLastSector(p.fileSystem().lastSector() - sectorAlignment(d));
	}

	return rval;
}

bool PartitionAlignment::alignChildren(const Device& d, Partition& p)
{
	bool rval = false;

	// In an extended partition we also need to align unallocated children at the beginning and at the end
	// (there should never be a need to align non-unallocated children)
	if (p.roles().has(PartitionRole::Extended))
	{
		if (p.children().size() > 0)
		{
			if (p.children().first()->roles().has(PartitionRole::Unallocated))
			{
				rval = true;
				p.children().first()->setFirstSector(p.firstSector() + d.sectorsPerTrack());
				p.children().first()->fileSystem().setFirstSector(p.fileSystem().firstSector() + d.sectorsPerTrack());
			}

			if (p.children().last()->roles().has(PartitionRole::Unallocated))
			{
				rval = true;
				p.children().last()->setLastSector(p.lastSector());
				p.children().last()->fileSystem().setLastSector(p.fileSystem().lastSector());
			}
		}
	}

	return rval;
}

/** Checks if a Partition can be aligned to a given sector on a given Device.

	@see PartitionTable::alignPartition(), PartitionTable::isAligned()

	@param d the Device the Partition is on
	@param p the Partition to align
	@param s the sector to align to
	@return true if aligning to @p s is possible
*/
bool PartitionAlignment::canAlignToSector(const Device& d, const Partition& p, qint64 s) const
{
	Q_ASSERT(d.partitionTable());

	if (s < d.partitionTable()->firstUsable() || s >= d.partitionTable()->lastUsable())
		return false;

	const Partition* other = d.partitionTable()->findPartitionBySector(s, PartitionRole(PartitionRole::Any));

	if (other && other->roles().has(PartitionRole::Unallocated) &&
			((p.roles().has(PartitionRole::Logical) && other->roles().has(PartitionRole::Logical)) ||
			(p.roles().has(PartitionRole::Primary) && other->roles().has(PartitionRole::Primary))))
		other = NULL;

	return other == NULL || other == &p || other == originalPartition();
}
