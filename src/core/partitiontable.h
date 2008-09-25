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

#if !defined(PARTITIONTABLE__H)

#define PARTITIONTABLE__H

#include "core/partitionnode.h"
#include "core/partitionrole.h"

#include <QList>
#include <qglobal.h>

class Device;
class Partition;
class LibParted;

/** @brief The partition table (a.k.a Disk Label)

	PartitionTable represents a partition table (or disk label). The current implementation makes quite
	a number of assumptions that are only true for MSDOS disk labels.

	PartitionTable has child nodes that represent Partitions.

	@author vl@fidra.de
*/
class PartitionTable : public PartitionNode
{
	Q_DISABLE_COPY(PartitionTable)
	
	friend class LibParted;
	
	public:
		/** Partition flags as defined by libparted */
		enum Flag
		{
			FlagNone = 0,
			FlagBoot = 1,
			FlagRoot = 2,
			FlagSwap = 4,
			FlagHidden = 8,
			FlagRaid = 16,
			FlagLvm = 32,
			FlagLba = 64,
			FlagHpService = 128,
			FlagPalo = 256,
			FlagPrep = 512,
			FlagMsftReserved = 1024
		};

		Q_DECLARE_FLAGS(Flags, Flag)

	public:
		PartitionTable();
		~PartitionTable();

	public:
		PartitionNode* parent() { return NULL; } /**< @return always NULL for PartitionTable */
		const PartitionNode* parent() const { return NULL; } /**< @return always NULL for PartitionTable */
		
		bool isRoot() const { return true; } /**< @return always true for PartitionTable */
		bool isReadOnly() const { return m_ReadOnly; } /**< @return true if the PartitionTable is read only */

		Partitions& children() { return m_Children; } /**< @return the children in this PartitionTable */
		const Partitions& children() const { return m_Children; } /**< @return the children in this PartitionTable */
		
		void append(Partition* partition);
		
		qint64 freeSectorsBefore(const Partition& p) const;
		qint64 freeSectorsAfter(const Partition& p) const;

		bool hasExtended() const;
		Partition* extended();

		PartitionRole::Roles childRoles(const Partition& p) const;

		int numPrimaries() const;
		int maxPrimaries() const { return m_MaxPrimaries; } /**< @return max number of primary partitions this PartitionTable can handle */

		const QString& typeName() const { return m_TypeName; } /**< @return the name of this PartitionTable type according to libparted */
		
		void updateUnallocated(const Device& d);
		void insertUnallocated(const Device& d, PartitionNode* p, qint64 start) const;

		static QList<Flag> flagList();
		static QString flagName(Flag f);
		static QStringList flagNames(Flags f);

		static void removeUnallocated(PartitionNode* p);
		void removeUnallocated();

		static bool isSnapped(const Device& d, const Partition& p);
		static bool snap(const Device& d, Partition& p, const Partition* originalPartition = NULL);

	protected:
		void clear();
		void setMaxPrimaries(qint32 n) { m_MaxPrimaries = n; }
		void setTypeName(const QString& s);
		void setReadOnly(bool b) { m_ReadOnly = b; }

	private:
		Partitions m_Children;
		qint32 m_MaxPrimaries;
		QString m_TypeName;
		bool m_ReadOnly;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PartitionTable::Flags)


#endif

