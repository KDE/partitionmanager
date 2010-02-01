/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
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

/** @file
*/

#include "core/partitiontable.h"
#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/globallog.h"

#include <kdebug.h>
#include <klocale.h>

/** Creates a new PartitionTable object with type MSDOS
	@param type name of the PartitionTable type (e.g. "msdos" or "gpt")
*/
PartitionTable::PartitionTable(LabelType type, qint64 first_usable, qint64 last_usable) :
	PartitionNode(),
	m_Children(),
	m_MaxPrimaries(maxPrimariesForLabelType(type)),
	m_Type(type),
	m_FirstUsable(first_usable),
	m_LastUsable(last_usable)
{
}

/** Destroys a PartitionTable object, destroying all children */
PartitionTable::~PartitionTable()
{
	clearChildren();
}

/** Gets the number of free sectors before a given child Partition in this PartitionTable.

	@param p the Partition for which to get the free sectors before
	@returns the number of free sectors before the Partition
*/
qint64 PartitionTable::freeSectorsBefore(const Partition& p) const
{
	const Partition* pred = predecessor(p);

	if (pred && pred->roles().has(PartitionRole::Unallocated))
		return pred->length();

	return 0;
}

/** Gets the number of free sectors after a given child Partition in this PartitionTable.

	@param p the Partition for which to get the free sectors after
	@returns the number of free sectors after the Partition
*/
qint64 PartitionTable::freeSectorsAfter(const Partition& p) const
{
	const Partition* succ = successor(p);

	if (succ && succ->roles().has(PartitionRole::Unallocated))
		return succ->length();

	return 0;
}

/** @return true if the PartitionTable has an extended Partition */
bool PartitionTable::hasExtended() const
{
	for (int i = 0; i < children().size(); i++)
		if (children()[i]->roles().has(PartitionRole::Extended))
			return true;

	return false;
}

/** @return pointer to the PartitionTable's extended Partition or NULL if none exists */
Partition* PartitionTable::extended()
{
	for (int i = 0; i < children().size(); i++)
		if (children()[i]->roles().has(PartitionRole::Extended))
			return children()[i];

	return NULL;
}

/** Gets valid PartitionRoles for a Partition
	@param p the Partition
	@return valid roles for the given Partition
*/
PartitionRole::Roles PartitionTable::childRoles(const Partition& p) const
{
	Q_ASSERT(p.parent());

	PartitionRole::Roles r = p.parent()->isRoot() ? PartitionRole::Primary : PartitionRole::Logical;

	if (r == PartitionRole::Primary && hasExtended() == false && diskLabelSupportsExtended(type()))
		r |= PartitionRole::Extended;

	return r;
}

/** @return the number of primaries in this PartitionTable */
int PartitionTable::numPrimaries() const
{
	int result = 0;

	foreach(const Partition* p, children())
		if (p->roles().has(PartitionRole::Primary) || p->roles().has(PartitionRole::Extended))
			result++;

	return result;
}

/** Appends a Partition to this PartitionTable
	@param partition pointer of the partition to append. Must not be NULL.
*/
void PartitionTable::append(Partition* partition)
{
	children().append(partition);
}

/** @param f the flag to get the name for
	@returns the flags name or an empty QString if the flag is not known
*/
QString PartitionTable::flagName(Flag f)
{
	switch(f)
	{
		case PartitionTable::FlagBoot: return i18nc("@item partition flag", "boot");
		case PartitionTable::FlagRoot: return i18nc("@item partition flag", "root");
		case PartitionTable::FlagSwap: return i18nc("@item partition flag", "swap");
		case PartitionTable::FlagHidden: return i18nc("@item partition flag", "hidden");
		case PartitionTable::FlagRaid: return i18nc("@item partition flag", "raid");
		case PartitionTable::FlagLvm: return i18nc("@item partition flag", "lvm");
		case PartitionTable::FlagLba: return i18nc("@item partition flag", "lba");
		case PartitionTable::FlagHpService: return i18nc("@item partition flag", "hpservice");
		case PartitionTable::FlagPalo: return i18nc("@item partition flag", "palo");
		case PartitionTable::FlagPrep: return i18nc("@item partition flag", "prep");
		case PartitionTable::FlagMsftReserved: return i18nc("@item partition flag", "msft-reserved");

		default:
			break;
	}

	return QString();
}

/** @return list of all flags */
QList<PartitionTable::Flag> PartitionTable::flagList()
{
	QList<PartitionTable::Flag> rval;

	rval.append(PartitionTable::FlagBoot);
	rval.append(PartitionTable::FlagRoot);
	rval.append(PartitionTable::FlagSwap);
	rval.append(PartitionTable::FlagHidden);
	rval.append(PartitionTable::FlagRaid);
	rval.append(PartitionTable::FlagLvm);
	rval.append(PartitionTable::FlagLba);
	rval.append(PartitionTable::FlagHpService);
	rval.append(PartitionTable::FlagPalo);
	rval.append(PartitionTable::FlagPrep);
	rval.append(PartitionTable::FlagMsftReserved);

	return rval;
}

/** @param flags the flags to get the names for
	@returns QStringList of the flags' names
*/
QStringList PartitionTable::flagNames(Flags flags)
{
	QStringList rval;

	int f = 1;

	QString s;
	while(!(s = flagName(static_cast<PartitionTable::Flag>(f))).isEmpty())
	{
		if (flags & f)
			rval.append(s);

		f <<= 1;
	}

	return rval;
}

/** Checks if a given Partition on a given Device is snapped to cylinder boundaries.

	Will print warning messages to GlobalLog if the Partition's first sector is not snapped and
	another one if the last sector is not snapped.

	@see snap(), canSnapToSector()

	@param d the Device the Partition is on
	@param p the Partition to check
	@return true if snapped
*/
bool PartitionTable::isSnapped(const Device& d, const Partition& p)
{
	// only msdos gets snapped -- TODO: add msdos_vista
	if (d.partitionTable()->type() != msdos)
		return true;

	// don't bother with unallocated space here.
	if (p.roles().has(PartitionRole::Unallocated))
		return true;

	qint64 delta = 0;

	// There are some special cases for snapping partitions to cylinder boundaries, apparently.
	// 1) If an extended partition starts at the beginning of the device (that would be sector 63
	// on modern drives, equivalent to sectorsPerTrack() in any case), the first logical partition
	// at the beginning of this extended partition starts at 2 * sectorsPerTrack().
	// 2) If a primary or extended starts at the beginning of a device, it starts at sectorsPerTrack().
	// 3) Any logical partition is always preceded by the extended partition table entry in the
	// sectorsPerTrack() before it, so it's always sectorsPerTrack() "late"
	if (p.roles().has(PartitionRole::Logical) && p.firstSector() == 2 * d.sectorsPerTrack())
		delta = (p.firstSector() - (2 * d.sectorsPerTrack())) % d.cylinderSize();
	else if (p.roles().has(PartitionRole::Logical) || p.firstSector() == d.sectorsPerTrack())
		delta = (p.firstSector() - d.sectorsPerTrack()) % d.cylinderSize();
	else
		delta = p.firstSector() % d.cylinderSize();

	bool rval = true;

	if (delta)
	{
		Log(Log::warning) << i18nc("@info/plain", "Partition <filename>%1</filename> does not start at a cylinder boundary (first sector: %2, modulo: %3).", p.deviceNode(), p.firstSector(), delta);
		rval = false;
	}

	delta = (p.lastSector() + 1) % d.cylinderSize();

	if (delta)
	{
		Log(Log::warning) << i18nc("@info/plain", "Partition <filename>%1</filename> does not end at a cylinder boundary (last sector: %2, modulo: %3).", p.deviceNode(), p.lastSector(), delta);
		rval = false;
	}

	return rval;
}

/** Checks if a Partition can be snapped to a given sector on a given Device.

	@see PartitionTable::snap(), PartitionTable::isSnapped()

	@param d the Device the Partition is on
	@param p the Partition to snap
	@param s the sector to snap to
	@param originalPartition pointer to another Partition @p p has just been copied from or NULL
	@return true if snapping to @p s is possible
*/
static bool canSnapToSector(const Device& d, const Partition& p, qint64 s, const Partition* originalPartition)
{
	Q_ASSERT(d.partitionTable());

	if (s < d.partitionTable()->firstUsable() || s >= d.partitionTable()->lastUsable())
		return false;

	const Partition* other = d.partitionTable()->findPartitionBySector(s, PartitionRole(PartitionRole::Logical | PartitionRole::Primary | PartitionRole::Extended | PartitionRole::Unallocated));

	if (other && other->roles().has(PartitionRole::Unallocated))
		other = NULL;

	return other == NULL || other == &p || other == originalPartition;
}

/** Snaps the given Partition on the given Device to cylinder boundaries.

	Tries under all accounts to keep the Partition's length equal to the original length or
	to increase it, if that is not possible. Will print a warning message to GlobalLog if
	this is not possible.

	The parameter @p originalPartition is required for cases where a Partition has just been
	duplicated to resize or move it. This method needs to know the original because of course
	the original does not prevent snapping to any sector allocated by it.

	@see canSnapToSector(), isSnapped()

	@param d the Device the Partition is on
	@param p the Partition to snap
	@param originalPartition pointer to a Partition object @p p has just been copied from or NULL
	@return true if Partition is now snapped to cylinder boundaries
*/
bool PartitionTable::snap(const Device& d, Partition& p, const Partition* originalPartition)
{
	// TODO: implement snapping to 2048-sector-boundaries for msdos_vista
	if (d.partitionTable()->type() != msdos)
		return true;

	const qint64 originalLength = p.length();
	qint64 delta = 0;
	bool lengthIsSnapped = false;

	// This is the same as in isSnapped(), only we additionally have to remember if the
	// partition's _length_ is "snapped", so to speak (i.e., evenly divisable by
	// the cylinder size)
	if (p.roles().has(PartitionRole::Logical) && p.firstSector() == 2 * d.sectorsPerTrack())
	{
		delta = (p.firstSector() - (2 * d.sectorsPerTrack())) % d.cylinderSize();
		lengthIsSnapped = (p.length() + (2 * d.sectorsPerTrack())) % d.cylinderSize() == 0;
	}
	else if (p.roles().has(PartitionRole::Logical) || p.firstSector() == d.sectorsPerTrack())
	{
		delta = (p.firstSector() - d.sectorsPerTrack()) % d.cylinderSize();
		lengthIsSnapped = (p.length() + d.sectorsPerTrack()) % d.cylinderSize() == 0;
	}
	else
	{
		delta = p.firstSector() % d.cylinderSize();
		lengthIsSnapped = p.length() % d.cylinderSize() == 0;
	}

	if (delta)
	{
		/** @todo Don't assume we always want to snap to the front.
			Always trying to snap to the front solves the problem that a partition does
			get too small to take another one that's copied to it, but it introduces
			a new bug: The user might create a partition aligned at the end of a device,
			extended partition or at the start of the next one, but we snap to the back
			and leave some space in between.
		*/
		// We always want to make the partition larger, not smaller. Making it smaller
		// might, in case it's a partition that another is being copied to, mean the partition
		// ends up too small. So try to move the start to the front first.
		qint64 snappedFirst = p.firstSector() - delta;

		// If we're now before the first usable sector, just take the first usable sector. This
		// will happen if we're already below cylinder one and snap to the front
		if (snappedFirst < d.partitionTable()->firstUsable())
			snappedFirst = d.partitionTable()->firstUsable();

		// Now if the cylinder boundary at the front is occupied...
		if (!canSnapToSector(d, p, snappedFirst, originalPartition))
		{
			// ... move to the cylinder towards the end of the device ...
			snappedFirst = p.firstSector() - delta + d.cylinderSize();

			// ... and move the end of the partition towards the end, too, if that is possible.
			// By doing this, we still try to keep the length >= the original length. If the
			// last sector ends up not being on a cylinder boundary by doing so, the code
			// below will deal with that.
			qint64 numTooShort = d.cylinderSize() - delta;
			if (canSnapToSector(d, p, p.lastSector() + numTooShort, originalPartition))
			{
				p.setLastSector(p.lastSector() + numTooShort);
				p.fileSystem().setLastSector(p.fileSystem().lastSector() + numTooShort);
			}
		}

		p.setFirstSector(snappedFirst);
		p.fileSystem().setFirstSector(snappedFirst);
	}

	delta = (p.lastSector() + 1) % d.cylinderSize();

	if (delta)
	{
		// Try to snap to the back first...
		qint64 snappedLast = p.lastSector() + d.cylinderSize() - delta;

		// .. but if we can retain the partition length exactly by snapping to the front ...
		if (lengthIsSnapped && p.length() - originalLength == delta)
			snappedLast -= d.cylinderSize();
		// ... or if there's something there already, snap to the front.
		else if (!canSnapToSector(d, p, snappedLast, originalPartition))
			snappedLast -= d.cylinderSize();

		p.setLastSector(snappedLast);
		p.fileSystem().setLastSector(snappedLast);
	}

	// Now, did we make the partition too big for its file system?
	while (p.length() > originalLength && p.capacity() > p.fileSystem().maxCapacity() && canSnapToSector(d, p, p.lastSector() - d.cylinderSize(), originalPartition))
	{
		p.setLastSector(p.lastSector() - d.cylinderSize());
		p.fileSystem().setLastSector(p.fileSystem().lastSector() - d.cylinderSize());
	}

	if (p.length() < originalLength)
		Log(Log::warning) <<  i18ncp("@info/plain", "The partition cannot be created with the requested length of 1 sector, ", "The partition cannot be created with the requested length of %1 sectors, ", originalLength)
                                    + i18ncp("@info/plain", "and will instead only be 1 sector long.", "and will instead only be %1 sectors long.", p.length());

	// In an extended partition we also need to snap unallocated children at the beginning and at the end
	// (there should never be a need to snap non-unallocated children)
	if (p.roles().has(PartitionRole::Extended))
	{
		if (p.children().size() > 0)
		{
			if (p.children().first()->roles().has(PartitionRole::Unallocated))
			{
				p.children().first()->setFirstSector(p.firstSector() + d.sectorsPerTrack());
				p.children().first()->fileSystem().setFirstSector(p.fileSystem().firstSector() + d.sectorsPerTrack());
			}

			if (p.children().last()->roles().has(PartitionRole::Unallocated))
			{
				p.children().last()->setLastSector(p.lastSector());
				p.children().last()->fileSystem().setLastSector(p.fileSystem().lastSector());
			}
		}
	}

	return isSnapped(d, p);
}

/** Creates a new unallocated Partition on the given Device.
	@param device the Device to create the new Partition on
	@param parent the parent PartitionNode for the new Partition
	@param start the new Partition's start sector
	@param end the new Partition's end sector
	@return pointer to the newly created Partition object or NULL if the Partition could not be created
*/
Partition* createUnallocated(const Device& device, PartitionNode& parent, qint64 start, qint64 end)
{
	PartitionRole::Roles r = PartitionRole::Unallocated;

	if (!parent.isRoot())
	{
		Partition* extended = dynamic_cast<Partition*>(&parent);

		if (extended == NULL)
		{
			kWarning() << "extended is null. start: " << start << ", end: " << end << ", device: " << device.deviceNode();
			return NULL;
		}

		// Leave a track free at the start for a new partition's metadata
		start += device.sectorsPerTrack();

		// .. and also at the end for the metadata for a partition to follow us, if we're not
		// at the end of the extended partition
		if (end < extended->lastSector())
			end -= device.sectorsPerTrack();

		r |= PartitionRole::Logical;
	}

	// TODO: what happens with this for non-msdos disk labels?
	if (end - start + 1 < device.cylinderSize())
		return NULL;

	return new Partition(&parent, device, PartitionRole(r), FileSystemFactory::create(FileSystem::Unknown, start, end), start, end, -1);
}

/** Removes all unallocated children from a PartitionNode
	@param p pointer to the parent to remove unallocated children from
*/
void PartitionTable::removeUnallocated(PartitionNode* p)
{
	Q_ASSERT(p != NULL);

	qint32 i = 0;

	while (i < p->children().size())
	{
		Partition* child = p->children()[i];

		if (child->roles().has(PartitionRole::Unallocated))
		{
			p->remove(child);
			delete child;
			continue;
		}

		if (child->roles().has(PartitionRole::Extended))
			removeUnallocated(child);

		i++;
	}
}

/**
	@overload
*/
void PartitionTable::removeUnallocated()
{
	removeUnallocated(this);
}

/** Inserts unallocated children for a Device's PartitionTable with the given parent.

	This method inserts unallocated Partitions for a parent, usually the Device this
	PartitionTable is on. It will also insert unallocated Partitions in any extended
	Partitions it finds.

	@warning This method assumes that no unallocated Partitions exist when it is called.

	@param d the Device this PartitionTable and @p p are on
	@param p the parent PartitionNode (may be this or an extended Partition)
	@param start the first sector to begin looking for free space
*/
void PartitionTable::insertUnallocated(const Device& d, PartitionNode* p, qint64 start) const
{
	Q_ASSERT(p != NULL);

	qint64 lastEnd = start;

	foreach (Partition* child, p->children())
	{
		p->insert(createUnallocated(d, *p, lastEnd, child->firstSector() - 1));

		if (child->roles().has(PartitionRole::Extended))
			insertUnallocated(d, child, child->firstSector());

		lastEnd = child->lastSector() + 1;
	}

	// Take care of the free space between the end of the last child and the end
	// of the device or the extended partition.
	qint64 parentEnd = lastUsable();

	if (!p->isRoot())
	{
		Partition* extended = dynamic_cast<Partition*>(p);
		Q_ASSERT(extended != NULL);
		parentEnd = (extended != NULL) ? extended->lastSector() : -1;
	}

	if (parentEnd >= firstUsable())
		p->insert(createUnallocated(d, *p, lastEnd, parentEnd));
}

/** Updates the unallocated Partitions for this PartitionTable.
	@param d the Device this PartitionTable is on
*/
void PartitionTable::updateUnallocated(const Device& d)
{
	removeUnallocated();
	insertUnallocated(d, this, firstUsable());
}

qint64 PartitionTable::defaultFirstUsable(const Device& d, LabelType t)
{
	if (t == msdos_vista)
		return 2048;

	return d.sectorsPerTrack() - 1;
}

qint64 PartitionTable::defaultLastUsable(const Device& d, LabelType t)
{
	if (t == gpt)
		return d.totalSectors() - 1 - 32 - 1;

	return d.totalSectors() - 1;
}

static struct
{
	const char* name; /**< name of disk label in libparted */
	quint32 maxPrimaries; /**< max numbers of primary partitions supported */
	bool canHaveExtended; /**< does disk label support extended partitions */
	bool isReadOnly; /**< does KDE Partition Manager support this only in read only mode */
	PartitionTable::LabelType type; /**< enum type */
} diskLabels[] =
{
	{ "aix", 4, false, true, PartitionTable::aix },
	{ "bsd", 8, false, true, PartitionTable::bsd },
	{ "dasd", 1, false, true, PartitionTable::dasd },
	{ "msdos", 4, true, false, PartitionTable::msdos },
	{ "msdos (vista)", 4, true, false, PartitionTable::msdos_vista },
	{ "dvh", 16, true, true, PartitionTable::dvh },
	{ "gpt", 128, false, false, PartitionTable::gpt },
	{ "loop", 1, false, true, PartitionTable::loop },
	{ "mac", 0xffff, false, true, PartitionTable::mac },
	{ "pc98", 16, false, true, PartitionTable::pc98 },
	{ "amiga", 128, false, true, PartitionTable::amiga },
	{ "sun", 8, false, true, PartitionTable::sun }
};

PartitionTable::LabelType PartitionTable::nameToLabelType(const QString& n)
{
	for (size_t i = 0; i < sizeof(diskLabels) / sizeof(diskLabels[0]); i++)
		if (n == diskLabels[i].name)
			return diskLabels[i].type;

	return PartitionTable::unknownLabel;
}

QString PartitionTable::labelTypeToName(LabelType l)
{
	for (size_t i = 0; i < sizeof(diskLabels) / sizeof(diskLabels[0]); i++)
		if (l == diskLabels[i].type)
			return diskLabels[i].name;

	return i18nc("@item/plain disk label name", "unknown");
}

qint64 PartitionTable::maxPrimariesForLabelType(LabelType l)
{
	for (size_t i = 0; i < sizeof(diskLabels) / sizeof(diskLabels[0]); i++)
		if (l == diskLabels[i].type)
			return diskLabels[i].maxPrimaries;

	return 1;
}

bool PartitionTable::diskLabelSupportsExtended(LabelType l)
{
	for (size_t i = 0; i < sizeof(diskLabels) / sizeof(diskLabels[0]); i++)
		if (l == diskLabels[i].type)
			return diskLabels[i].canHaveExtended;

	return false;
}

bool PartitionTable::diskLabelIsReadOnly(LabelType l)
{
	for (size_t i = 0; i < sizeof(diskLabels) / sizeof(diskLabels[0]); i++)
		if (l == diskLabels[i].type)
			return diskLabels[i].isReadOnly;

	return false;
}

bool PartitionTable::isVistaDiskLabel() const
{
	if (type() == PartitionTable::msdos)
	{
		const Partition* part = findPartitionBySector(2048, PartitionRole(PartitionRole::Primary));
		if (part && part->firstSector() == 2048)
			return true;
	}

	return false;
}

void PartitionTable::setType(LabelType t)
{
	// hack: if the type has been msdos and is now set to vista, make sure to also
	// set the first usable sector to 2048 now.
	if (type() == msdos && t == msdos_vista)
		setFirstUsableSector(2048);

	m_Type = t;
}

