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

#if !defined(FILESYSTEM__H)

#define FILESYSTEM__H

#include <qglobal.h>
#include <QStringList>
#include <QString>
#include <QList>

class Device;
class Report;

/** @brief Base class for all FileSystems.

	Represents a file system and handles support for various types of operations that can
	be performed on those.

	@author vl@fidra.de
 */
class FileSystem
{
	Q_DISABLE_COPY(FileSystem);
	
	public:
		/** Supported FileSystem types */
		enum Type
		{
			Unknown = 0,
			Extended,

			Ext2,
			Ext3,
			LinuxSwap,
			Fat16,
			Fat32,
			Ntfs,
			ReiserFS,
			Reiser4,
			Xfs,
			Jfs,
			Hfs,
			HfsPlus,
			Ufs,
			Unformatted,
			__lastType
		};

		/** The type of support for a given FileSystem action */
		enum SupportType
		{
			SupportNone = 0,		/**< no support */
			SupportInternal = 1,	/**< internal support */
			SupportLibParted = 2,	/**< supported by libparted */
			SupportExternal = 4		/**< supported by some external command */
		};

		Q_DECLARE_FLAGS(SupportTypes, SupportType);

	protected:
		FileSystem(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type t);

	public:
		virtual ~FileSystem() {}

	public:
		virtual qint64 readUsedCapacity(const QString& deviceNode) const;
		virtual QString readLabel(const QString& deviceNode) const;
		virtual bool create(Report& report, const QString& deviceNode) const;
		virtual bool resize(Report& report, const QString& deviceNode, qint64 newLength) const;
		virtual bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel);
		virtual bool copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const;
		virtual bool backup(Report& report, const Device& sourceDevice, const QString& deviceNode, const QString& filename) const;
		virtual bool check(Report& report, const QString& deviceNode) const;
		virtual bool updateUUID(Report& report, const QString& deviceNode) const;

		virtual SupportType supportGetUsed() const { return SupportNone; } /**< @return SupportType for getting used capacity */
		virtual SupportType supportGetLabel() const { return SupportNone; } /**< @return SupportType for reading label*/
		virtual SupportType supportCreate() const { return SupportNone; } /**< @return SupportType for creating */
		virtual SupportType supportGrow() const { return SupportNone; } /**< @return SupportType for growing */
		virtual SupportType supportShrink() const { return SupportNone; } /**< @return SupportType for shrinking */
		virtual SupportType supportMove() const { return SupportNone; } /**< @return SupportType for moving */
		virtual SupportType supportCheck() const { return SupportNone; } /**< @return SupportType for checking */
		virtual SupportType supportCopy() const { return SupportNone; } /**< @return SupportType for copying */
		virtual SupportType supportBackup() const { return SupportNone; } /**< @return SupportType for backing up */
		virtual SupportType supportSetLabel() const { return SupportNone; } /**< @return SupportType for setting label*/
		virtual SupportType supportUpdateUUID() const { return SupportNone; } /**< @return SupportType for updating the UUID */

		virtual qint64 minCapacity() const;
		virtual qint64 maxCapacity() const;

		virtual QString name() const;
		virtual FileSystem::Type type() const { return m_Type; } /**< @return the FileSystem's type */

		static QString nameForType(FileSystem::Type t);
		static QList<FileSystem::Type> types();
		static FileSystem::Type typeForName(const QString& s);
		static FileSystem::Type defaultFileSystem() { return Ext2; } /**< @return the default FileSystem type */

		virtual bool canMount(const QString&) const { return false; } /**< @return true if this FileSystem can be mounted */
		virtual bool canUnmount(const QString&) const { return false; } /**< @return true if this FileSystem can be unmounted */
		
		virtual QString mountTitle() const;
		virtual QString unmountTitle() const;
		
		virtual bool mount(const QString& mountPoint);
		virtual bool unmount(const QString& mountPoint);

		qint64 firstSector() const { return m_FirstSector; } /**< @return the FileSystem's first sector */
		qint64 lastSector() const { return m_LastSector; } /**< @return the FileSystem's last sector */
		qint64 length() const { return lastSector() - firstSector() + 1; } /**< @return the FileSystem's length */
		
		void setFirstSector(qint64 s) { m_FirstSector = s; } /**< @param s the new first sector */
		void setLastSector(qint64 s) { m_LastSector = s; } /**< @param s the new last sector */

		void move(qint64 newStartSector);
		
		const QString& label() const { return m_Label; } /**< @return the FileSystem's label */
		qint64 sectorsUsed() const { return m_SectorsUsed; } /**< @return the sectors in use on the FileSystem */
		
		void setSectorsUsed(qint64 s) { m_SectorsUsed = s; } /**< @param s the new value for sectors in use */
		void setLabel(const QString& s) { m_Label = s; } /**< @param s the new label */
		
	protected:
		static bool findExternal(const QString& cmdName, const QStringList& args = QStringList(), int exptectedCode = 1);

	protected:
		FileSystem::Type m_Type;
		qint64 m_FirstSector;
		qint64 m_LastSector;
		qint64 m_SectorsUsed;
		QString m_Label;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FileSystem::SupportTypes);

#endif
