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

#include "core/partition.h"

#include "core/device.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/externalcommand.h"
#include "util/report.h"

#include <QString>
#include <QStringList>

#include <kdebug.h>
#include <klocale.h>

/** Creates a new Partition object.
	@param parent the Partition's parent. May be another Partition (for logicals) or a PartitionTable. Must not be NULL.
	@param device the Device this Partition is on.
	@param role the Partition's role(s)
	@param fs pointer to the Partition's FileSystem object. The Partition object will take ownership of this.
	@param sectorStart the first sector of the Partition on its Device
	@param sectorEnd the last sector of the Partition on its Device
	@param number the Partition's device number, e.g. 7 for /dev/sdd7
	@param availableFlags the flags available for this Partition according to libparted
	@param mountPoints string list of mount points found for this Partition
	@param mounted true if the Partition is mounted
	@param activeFlags active flags for this Partition according to libparted
	@param state the Partition's state
*/
Partition::Partition(PartitionNode* parent, const Device& device, const PartitionRole& role, FileSystem* fs, qint64 sectorStart, qint64 sectorEnd, qint32 number, PartitionTable::Flags availableFlags, const QStringList& mountPoints, bool mounted, PartitionTable::Flags activeFlags, State state) :
	PartitionNode(),
	m_Number(number),
	m_Children(),
	m_Parent(parent),
	m_FileSystem(fs),
	m_Roles(role),
	m_FirstSector(sectorStart),
	m_LastSector(sectorEnd),
	m_DevicePath(device.deviceNode()),
	m_MountPoints(mountPoints),
	m_AvailableFlags(availableFlags),
	m_ActiveFlags(activeFlags),
	m_IsMounted(mounted),
	m_SectorSize(device.sectorSize()),
	m_State(state)
{
	Q_ASSERT(m_Parent);
}

/** Destroys a Partition, destroying its children and its FileSystem */
Partition::~Partition()
{
	clearChildren();
	deleteFileSystem();
}

/**	@param other Partition to copy
*/
Partition::Partition(const Partition& other) :
	PartitionNode(),
	m_Number(other.m_Number),
	m_Children(),
	m_Parent(other.m_Parent),
	m_FileSystem(FileSystemFactory::create(other.fileSystem())),
	m_Roles(other.m_Roles),
	m_FirstSector(other.m_FirstSector),
	m_LastSector(other.m_LastSector),
	m_DevicePath(other.m_DevicePath),
	m_MountPoints(other.m_MountPoints),
	m_AvailableFlags(other.m_AvailableFlags),
	m_ActiveFlags(other.m_ActiveFlags),
	m_IsMounted(other.m_IsMounted),
	m_SectorSize(other.m_SectorSize),
	m_State(other.m_State)
{
	foreach(const Partition* child, other.children())
	{
		Partition* p = new Partition(*child);
		p->setParent(this);
		m_Children.append(p);
	}
}

/** @param other Partition to assign from */
Partition& Partition::operator=(const Partition& other)
{
	if (&other == this)
		return *this;

	clearChildren();

	foreach(const Partition* child, other.children())
	{
		Partition* p = new Partition(*child);
		p->setParent(this);
		m_Children.append(p);
	}

	m_Number = other.m_Number;
	m_FileSystem = FileSystemFactory::create(other.fileSystem());
	m_Roles = other.m_Roles;
	m_FirstSector = other.m_FirstSector;
	m_LastSector = other.m_LastSector;
	m_DevicePath = other.m_DevicePath;
	m_MountPoints = other.m_MountPoints;
	m_AvailableFlags = other.m_AvailableFlags;
	m_ActiveFlags = other.m_ActiveFlags;
	m_IsMounted = other.m_IsMounted;
	m_SectorSize = other.m_SectorSize;
	m_State = other.m_State;

	return *this;
}

/** @return a short descriptive text or, in case the Partition has StateNone, its device node. */
QString Partition::deviceNode() const
{
	if (roles().has(PartitionRole::None) || roles().has(PartitionRole::Unallocated))
		return i18nc("@item partition name", "unallocated");

	if (state() == StateNew)
		return i18nc("@item partition name", "New Partition");

	if (state() == StateRestore)
		return i18nc("@item partition name", "Restored Partition");

	QString res = m_DevicePath + QString::number(number());

	if (state() == StateCopy)
		return i18nc("@item partition name", "Copy of %1", res);

	return res;
}

/** @return the sectors used in the Partition's FileSystem or, in case of an extended partition, the sum of used sectors of the Partition's children */
qint64 Partition::sectorsUsed() const
{
	if (!roles().has(PartitionRole::Extended))
		return fileSystem().sectorsUsed();

	qint64 result = 0;
	foreach (const Partition* p, children())
		if (!p->roles().has(PartitionRole::Unallocated))
			result += p->length();

	return result;
}

/** @return the minimum number of sectors this Partition must be long */
qint64 Partition::minimumSectors() const
{
	return fileSystem().minCapacity() / sectorSize();
}

/** @return the maximum number of sectors this Partition may be long */
qint64 Partition::maximumSectors() const
{
	return fileSystem().maxCapacity() / sectorSize();
}

/** Adjusts the numbers of logical Partitions for an extended Partition.

	This is required if a logical Partition is deleted or inserted because logicals must be numberd from
	5 onwards without a gap. So if the user deletes Partition number 7 and there is a number 8, 8 becomes the
	"new" 7. And since this happens somewhere in the middle of a DeleteOperation, we have to adjust to that so the
	next Job still finds the Partition it wants to deal with.

	@param deletedNumber the number of a deleted logical or -1 if none has been deleted
	@param insertedNumber the number of an inserted logical or -1 if none has been inserted
*/
void Partition::adjustLogicalNumbers(qint32 deletedNumber, qint32 insertedNumber)
{
	if (!roles().has(PartitionRole::Extended))
		return;

	foreach (Partition* p, children())
	{
		if (deletedNumber > 4 && p->number() > deletedNumber)
			p->setNumber(p->number() - 1);
		else if (insertedNumber > 4 && p->number() >= insertedNumber)
			p->setNumber(p->number() + 1);
	}
}

/** @return the highest sector number an extended Partition can begin at */
qint64 Partition::maxFirstSector() const
{
	qint64 rval = -1;

	foreach (const Partition* child, children())
		if (!child->roles().has(PartitionRole::Unallocated) && (child->firstSector() < rval || rval == -1))
			rval = child->firstSector();

	return rval;
}

/** @return the lowest sector number an extended Partition can end at */
qint64 Partition::minLastSector() const
{
	qint64 rval = -1;

	foreach (const Partition* child, children())
		if (!child->roles().has(PartitionRole::Unallocated) && child->lastSector() > rval)
			rval = child->lastSector();

	return rval;
}

/** @return true if the Partition has children */
bool Partition::hasChildren() const
{
	foreach (const Partition* child, children())
		if (!child->roles().has(PartitionRole::Unallocated))
			return true;

	return false;
}

/** Sets an extended Partition to mounted if any of its children are mounted */
void Partition::checkChildrenMounted()
{
	setMounted(isChildMounted());
}

/** @return true if this Partition can be mounted */
bool Partition::canMount() const
{
	// cannot mount if already mounted
	if (isMounted())
		return false;

	// if the file system says we can mount without mount points, that's fine
	// (this is the case for swap only, actually)
	if (fileSystem().canMount(deviceNode()))
		return true;

	// cannot mount if we have no mount points
	return !mountPoints().isEmpty();
}

/** @return true if this Partition can be unmounted */
bool Partition::canUnmount() const
{
	return !roles().has(PartitionRole::Extended) && isMounted();
}

/** Tries to mount a Partition.
	@return true on success
*/
bool Partition::mount(Report& report)
{
	if (isMounted())
		return false;

	bool success = false;

	if (fileSystem().canMount(deviceNode()))
		success = fileSystem().mount(deviceNode());
	else
	{
		foreach(const QString& mp, mountPoints())
		{
			ExternalCommand mountCmd(report, "mount", QStringList() << "-v" << deviceNode() << mp);
			if (mountCmd.run() && mountCmd.exitCode() == 0)
				success = true;
		}
	}

	setMounted(success);

	return success;
}

/** Tries to unmount a Partition.
	@return true on success
*/
bool Partition::unmount(Report& report)
{
	if (!isMounted())
		return false;

	bool success = true;

	if (fileSystem().canUnmount(deviceNode()))
		success = fileSystem().unmount(deviceNode());
	else
	{
		foreach(const QString& mp, mountPoints())
		{
			ExternalCommand umountCmd(report, "umount", QStringList() << "-v" << mp);
			if (!umountCmd.run() || umountCmd.exitCode() != 0)
				success = false;
		}
	}

	setMounted(!success);

	return success;
}

void Partition::deleteFileSystem()
{
	delete m_FileSystem;
	m_FileSystem = NULL;
}

void Partition::setFileSystem(FileSystem* fs)
{
	m_FileSystem = fs;
}

void Partition::move(qint64 newStartSector)
{
	const qint64 savedLength = length();
	setFirstSector(newStartSector);
	setLastSector(newStartSector + savedLength - 1);
}
