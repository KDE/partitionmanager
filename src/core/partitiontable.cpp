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

#include <QFile>
#include <QTextStream>

#include <config.h>

/** Creates a new PartitionTable object with type MSDOS
	@param type name of the PartitionTable type (e.g. "msdos" or "gpt")
*/
PartitionTable::PartitionTable(TableType type, qint64 first_usable, qint64 last_usable) :
	PartitionNode(),
	m_Children(),
	m_MaxPrimaries(maxPrimariesForTableType(type)),
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

	if (r == PartitionRole::Primary && hasExtended() == false && tableTypeSupportsExtended(type()))
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

/** @return the sector size to align the partition start and end to
*/
qint64 PartitionTable::sectorAlignment(const Device& d)
{
	return d.partitionTable()->type() == PartitionTable::msdos ? d.cylinderSize() : Config::sectorAlignment();
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
bool PartitionTable::isAligned(const Device& d, const Partition& p)
{
	// don't bother with unallocated space here.
	if (p.roles().has(PartitionRole::Unallocated))
		return true;

	qint64 delta = 0;

	if (d.partitionTable()->type() == msdos)
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

/** Checks if a Partition can be aligned to a given sector on a given Device.

	@see PartitionTable::alignPartition(), PartitionTable::isAligned()

	@param d the Device the Partition is on
	@param p the Partition to align
	@param s the sector to align to
	@param originalPartition pointer to another Partition @p p has just been copied from or NULL
	@return true if aligning to @p s is possible
*/
static bool canAlignToSector(const Device& d, const Partition& p, qint64 s, const Partition* originalPartition)
{
	Q_ASSERT(d.partitionTable());

	if (s < d.partitionTable()->firstUsable() || s >= d.partitionTable()->lastUsable())
		return false;

	const Partition* other = d.partitionTable()->findPartitionBySector(s, PartitionRole(PartitionRole::Logical | PartitionRole::Primary | PartitionRole::Extended | PartitionRole::Unallocated));

	if (other && other->roles().has(PartitionRole::Unallocated))
		other = NULL;

	return other == NULL || other == &p || other == originalPartition;
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
	@return true if Partition is now aligned to cylinder boundaries
*/
bool PartitionTable::alignPartition(const Device& d, Partition& p, const Partition* originalPartition)
{
	const qint64 originalLength = p.length();
	qint64 delta = 0;
	bool lengthIsAligned = false;

	// This is the same as in isAligned(), only we additionally have to remember if the
	// partition's _length_ is "aligned", so to speak (i.e., evenly divisable by
	// the sectorAlignment()
	if (d.partitionTable()->type() == msdos)
	{
		if (p.roles().has(PartitionRole::Logical) && p.firstSector() == 2 * d.sectorsPerTrack())
		{
			delta = (p.firstSector() - (2 * d.sectorsPerTrack())) % sectorAlignment(d);
			lengthIsAligned = (p.length() + (2 * d.sectorsPerTrack())) % sectorAlignment(d) == 0;
		}
		else if (p.roles().has(PartitionRole::Logical) || p.firstSector() == d.sectorsPerTrack())
		{
			delta = (p.firstSector() - d.sectorsPerTrack()) % sectorAlignment(d);
			lengthIsAligned = (p.length() + d.sectorsPerTrack()) % sectorAlignment(d) == 0;
		}
		else
		{
			delta = p.firstSector() % sectorAlignment(d);
			lengthIsAligned = p.length() % sectorAlignment(d) == 0;
		}
	}
	else
	{
			delta = p.firstSector() % sectorAlignment(d);
			lengthIsAligned = p.length() % sectorAlignment(d) == 0;
	}

	if (delta)
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
		qint64 alignedFirst = p.firstSector() - delta;

		// If we're now before the first usable sector, just take the first usable sector. This
		// will happen if we're already below cylinder one and align to the front
		if (alignedFirst < d.partitionTable()->firstUsable())
			alignedFirst = d.partitionTable()->firstUsable();

		// Now if the cylinder boundary at the front is occupied...
		if (!canAlignToSector(d, p, alignedFirst, originalPartition))
		{
			// ... move to the cylinder towards the end of the device ...
			alignedFirst = p.firstSector() - delta + sectorAlignment(d);

			// ... and move the end of the partition towards the end, too, if that is possible.
			// By doing this, we still try to keep the length >= the original length. If the
			// last sector ends up not being on a cylinder boundary by doing so, the code
			// below will deal with that.
			qint64 numTooShort = sectorAlignment(d) - delta;
			if (canAlignToSector(d, p, p.lastSector() + numTooShort, originalPartition))
			{
				p.setLastSector(p.lastSector() + numTooShort);
				p.fileSystem().setLastSector(p.fileSystem().lastSector() + numTooShort);
			}
		}

		p.setFirstSector(alignedFirst);
		p.fileSystem().setFirstSector(alignedFirst);
	}

	delta = (p.lastSector() + 1) % sectorAlignment(d);

	if (delta)
	{
		// Try to align to the back first...
		qint64 alignedLast = p.lastSector() + sectorAlignment(d) - delta;

		// .. but if we can retain the partition length exactly by aligning to the front ...
		if (lengthIsAligned && p.length() - originalLength == delta)
			alignedLast -= sectorAlignment(d);
		// ... or if there's something there already, align to the front.
		else if (!canAlignToSector(d, p, alignedLast, originalPartition))
			alignedLast -= sectorAlignment(d);

		p.setLastSector(alignedLast);
		p.fileSystem().setLastSector(alignedLast);
	}

	// Now, did we make the partition too big for its file system?
	while (p.length() > originalLength && p.capacity() > p.fileSystem().maxCapacity() && canAlignToSector(d, p, p.lastSector() - sectorAlignment(d), originalPartition))
	{
		p.setLastSector(p.lastSector() - sectorAlignment(d));
		p.fileSystem().setLastSector(p.fileSystem().lastSector() - sectorAlignment(d));
	}

	// In an extended partition we also need to align unallocated children at the beginning and at the end
	// (there should never be a need to align non-unallocated children)
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

	return isAligned(d, p);
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

		// Leave a track (cylinder aligned) or sector alignment sectors (sector based) free at the
		// start for a new partition's metadata
		start += device.partitionTable()->type() == PartitionTable::msdos ? device.sectorsPerTrack() : PartitionTable::sectorAlignment(device);

		// .. and also at the end for the metadata for a partition to follow us, if we're not
		// at the end of the extended partition
		if (end < extended->lastSector())
			end -= device.partitionTable()->type() == PartitionTable::msdos ? device.sectorsPerTrack() : PartitionTable::sectorAlignment(device);

		r |= PartitionRole::Logical;
	}

	if (end - start + 1 < PartitionTable::sectorAlignment(device))
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

qint64 PartitionTable::defaultFirstUsable(const Device& d, TableType t)
{
	if (t == msdos && Config::useCylinderAlignment())
		return d.sectorsPerTrack();

	return Config::sectorAlignment();
}

qint64 PartitionTable::defaultLastUsable(const Device& d, TableType t)
{
	if (t == gpt)
		return d.totalSectors() - 1 - 32 - 1;

	return d.totalSectors() - 1;
}

static struct
{
	const char* name; /**< name of partition table type */
	quint32 maxPrimaries; /**< max numbers of primary partitions supported */
	bool canHaveExtended; /**< does partition table type support extended partitions */
	bool isReadOnly; /**< does KDE Partition Manager support this only in read only mode */
	PartitionTable::TableType type; /**< enum type */
} tableTypes[] =
{
	{ "aix", 4, false, true, PartitionTable::aix },
	{ "bsd", 8, false, true, PartitionTable::bsd },
	{ "dasd", 1, false, true, PartitionTable::dasd },
	{ "msdos", 4, true, false, PartitionTable::msdos },
	{ "msdos", 4, true, false, PartitionTable::msdos_sectorbased },
	{ "dvh", 16, true, true, PartitionTable::dvh },
	{ "gpt", 128, false, false, PartitionTable::gpt },
	{ "loop", 1, false, true, PartitionTable::loop },
	{ "mac", 0xffff, false, true, PartitionTable::mac },
	{ "pc98", 16, false, true, PartitionTable::pc98 },
	{ "amiga", 128, false, true, PartitionTable::amiga },
	{ "sun", 8, false, true, PartitionTable::sun }
};

PartitionTable::TableType PartitionTable::nameToTableType(const QString& n)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (n == tableTypes[i].name)
			return tableTypes[i].type;

	return PartitionTable::unknownTableType;
}

QString PartitionTable::tableTypeToName(TableType l)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (l == tableTypes[i].type)
			return tableTypes[i].name;

	return i18nc("@item/plain partition table name", "unknown");
}

qint64 PartitionTable::maxPrimariesForTableType(TableType l)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (l == tableTypes[i].type)
			return tableTypes[i].maxPrimaries;

	return 1;
}

bool PartitionTable::tableTypeSupportsExtended(TableType l)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (l == tableTypes[i].type)
			return tableTypes[i].canHaveExtended;

	return false;
}

bool PartitionTable::tableTypeIsReadOnly(TableType l)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (l == tableTypes[i].type)
			return tableTypes[i].isReadOnly;

	return false;
}

/** Simple heuristic to determine if the PartitionTable is sector aligned (i.e.
	if its Partitions begin at sectors evenly divisable by Config::sectorAlignment().
	@return true if is sector aligned, otherwise false
*/
bool PartitionTable::isSectorBased() const
{
	if (type() == PartitionTable::msdos)
	{
		// user has turned cylinder based alignment off and partition table is empty
		if (Config::useCylinderAlignment() == false && numPrimaries() == 0)
			return true;

		// if not all partitions start at a point evenly divisable by sectorAlignment it's
		// a cylinder-aligned msdos partition table
		foreach(const Partition* p, children())
			if (p->firstSector() % Config::sectorAlignment() != 0)
				return false;

		// must be sector aligned
		return true;
	}

	return false;
}

void PartitionTable::setType(const Device& d, TableType t)
{
	setFirstUsableSector(defaultFirstUsable(d, t));
	setLastUsableSector(defaultLastUsable(d, t));

	m_Type = t;
}

static bool isPartitionLessThan(const Partition* p1, const Partition* p2)
{
	return p1->number() < p2->number();
}

QTextStream& operator<<(QTextStream& stream, const PartitionTable& ptable)
{
	stream << "type: \"" << ptable.typeName() << "\"\n"
		<< "align: \"" << (ptable.type() == PartitionTable::msdos ? "cylinder" : "sector") << "\"\n"
		<< "\n# number start end type roles label flags\n";

	QList<const Partition*> partitions;

	foreach(const Partition* p, ptable.children())
		if (!p->roles().has(PartitionRole::Unallocated))
		{
			partitions.append(p);

			if (p->roles().has(PartitionRole::Extended))
				foreach(const Partition* child, p->children())
					if (!child->roles().has(PartitionRole::Unallocated))
						partitions.append(child);
		}

	qSort(partitions.begin(), partitions.end(), isPartitionLessThan);

	foreach(const Partition* p, partitions)
		stream << *p;

	return stream;
}
