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

qint64 PartitionAlignment::firstDelta(const Device& d, const Partition& p, qint64 s)
{
	if (d.partitionTable()->type() == PartitionTable::msdos)
	{
		if (p.roles().has(PartitionRole::Logical) && s == 2 * d.sectorsPerTrack())
			return (s - (2 * d.sectorsPerTrack())) % sectorAlignment(d);

		if (p.roles().has(PartitionRole::Logical) || s == d.sectorsPerTrack())
			return (s - d.sectorsPerTrack()) % sectorAlignment(d);
	}

	return s % sectorAlignment(d);
}

qint64 PartitionAlignment::lastDelta(const Device& d, const Partition&, qint64 s)
{
	return (s + 1) % sectorAlignment(d);
}

bool PartitionAlignment::isLengthAligned(const Device& d, const Partition& p)
{
	if (d.partitionTable()->type() == PartitionTable::msdos)
	{
		if (p.roles().has(PartitionRole::Logical) && p.firstSector() == 2 * d.sectorsPerTrack())
			return (p.length() + (2 * d.sectorsPerTrack())) % sectorAlignment(d) == 0;

		if (p.roles().has(PartitionRole::Logical) || p.firstSector() == d.sectorsPerTrack())
			return (p.length() + d.sectorsPerTrack()) % sectorAlignment(d) == 0;
	}

	return p.length() % sectorAlignment(d) == 0;
}

/** Aligns a Partition on a Device to the PartitionTable's required alignment.

	Tries under all accounts to keep the Partition's length equal to the original length or
	to increase it, if that is not possible. Will print a warning message to GlobalLog if
	this is not possible.

	@return true if Partition is now properly aligned
*/
bool PartitionAlignment::alignPartition(const Device& d, Partition& p)
{
	const qint64 originalLength = p.length();
	const bool originalLengthAligned = isLengthAligned(d, p);

	if (alignedFirstSector(d, p, p.firstSector()) > p.firstSector())
	{
		// If the aligned sector moves the start towards the end of the drive, move the
		// end of the partition towards the end, too, if that is possible, trying to keep the
		// partition's length greater than or equal to its original length.
		const qint64 numTooShort = alignedFirstSector(d, p, p.firstSector()) - p.firstSector();
		if (canAlignToSector(d, p, p.lastSector() + numTooShort))
		{
			p.setLastSector(p.lastSector() + numTooShort);
			p.fileSystem().setLastSector(p.fileSystem().lastSector() + numTooShort);
		}
	}

	p.setFirstSector(alignedFirstSector(d, p, p.firstSector()));
	p.fileSystem().setFirstSector(alignedFirstSector(d, p, p.firstSector()));

	p.setLastSector(alignedLastSector(d, p, p.lastSector(), originalLength, originalLengthAligned));
	p.fileSystem().setLastSector(alignedLastSector(d, p, p.lastSector(), originalLength, originalLengthAligned));

	checkAlignConstraints(d, p, originalLength);
	alignChildren(d, p);

	return isAligned(d, p);
}

/** Checks if the Partition is properly aligned to the PartitionTable's alignment requirements.

	Will print warning messages to GlobalLog if the Partition's first sector is not aligned and
	another one if the last sector is not aligned. This can be suppressed with setting the @p quiet to
	true.

	@see alignPartition(), canAlignToSector()

	@param d device the partition is on
	@param p the partition to check
	@param quiet if true, will not print warning
	@return true if propertly aligned
*/
bool PartitionAlignment::isAligned(const Device& d, const Partition& p, bool quiet)
{
	return isAligned(d, p, p.firstSector(), p.lastSector(), quiet);
}

bool PartitionAlignment::isAligned(const Device& d, const Partition& p, qint64 newFirst, qint64 newLast, bool quiet)
{
	if (firstDelta(d, p, newFirst) && !quiet)
		Log(Log::warning) << i18nc("@info/plain", "Partition <filename>%1</filename> is not properly aligned (first sector: %2, modulo: %3).", p.deviceNode(), newFirst, firstDelta(d, p, newFirst));

	if (lastDelta(d, p, newLast) && !quiet)
		Log(Log::warning) << i18nc("@info/plain", "Partition <filename>%1</filename> is not properly aligned (last sector: %2, modulo: %3).", p.deviceNode(), newLast, lastDelta(d, p, newLast));

	return firstDelta(d, p, newFirst) == 0 && lastDelta(d, p, newLast) == 0;
}

/** @return the sector size to align the partition start and end to
*/
qint64 PartitionAlignment::sectorAlignment(const Device& d)
{
	return d.partitionTable()->type() == PartitionTable::msdos ? d.cylinderSize() : Config::sectorAlignment();
}

qint64 PartitionAlignment::alignedFirstSector(const Device& d, const Partition& p, qint64 s)
{
	qint64 rval = s;

	if (firstDelta(d, p, s))
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
		rval = s - firstDelta(d, p, s);

		// If we're now before the first usable sector, just take the first usable sector.
		if (rval < d.partitionTable()->firstUsable())
			rval = d.partitionTable()->firstUsable();

		// Now if the alignment sector at the front is occupied  move towards the end of the device
		if (!canAlignToSector(d, p, rval))
			rval = s - firstDelta(d, p, s) + sectorAlignment(d);
	}

	return rval;
}

qint64 PartitionAlignment::alignedLastSector(const Device& d, const Partition& p, qint64 s, qint64 original_length, bool original_aligned)
{
	qint64 rval = s;

	if (lastDelta(d, p, s))
	{
		// Try to align to the back first...
		rval = s + sectorAlignment(d) - lastDelta(d, p, s);

		// .. but if we can retain the partition length exactly by aligning to the front ...
		if (original_aligned && p.length() - original_length == lastDelta(d, p, s))
			rval -= sectorAlignment(d);
		// ... or if there's something there already, align to the front.
		else if (!canAlignToSector(d, p, rval))
			rval -= sectorAlignment(d);
	}

	return rval;
}

bool PartitionAlignment::checkAlignConstraints(const Device& d, Partition& p, qint64 original_length)
{
	bool rval = false;

	// Now, did we make the partition too big for its file system?
	while ((original_length == -1 || p.length() > original_length) && p.capacity() > p.fileSystem().maxCapacity() && canAlignToSector(d, p, p.lastSector() - sectorAlignment(d)))
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

	// TODO: this assumes alignment == cylinder boundaries. is that correct inside extended partitions when
	// alignment isn't msdos legacy?

	// In an extended partition we also need to align unallocated children at the beginning and at the end
	// (there should never be a need to align non-unallocated children)
	if (p.roles().has(PartitionRole::Extended) && p.children().size() > 0)
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

	return rval;
}

bool PartitionAlignment::canAlignToSector(const Device& d, const Partition& p, qint64 s)
{
	Q_ASSERT(d.partitionTable());

	if (s < d.partitionTable()->firstUsable() || s >= d.partitionTable()->lastUsable())
		return false;

	const Partition* other = d.partitionTable()->findPartitionBySector(s, PartitionRole(PartitionRole::Any));

	// nothing found should not happen unless there is an extended boot record there inside an
	// extended partition
	if (other == NULL)
		return false;

	if (other && other->roles().has(PartitionRole::Unallocated) &&
			((p.roles().has(PartitionRole::Logical) && other->roles().has(PartitionRole::Logical)) ||
			(p.roles().has(PartitionRole::Primary) && other->roles().has(PartitionRole::Primary))))
		return true;

	return other == &p;
}
