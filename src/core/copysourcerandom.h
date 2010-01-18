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

#if !defined(COPYSOURCERANDOM__H)

#define COPYSOURCERANDOM__H

#include "core/copysource.h"

#include <QFile>

class CopyTarget;

/** @brief A source of random data to copy from.

	Represents a random date source to copy from. Used to securely overwrite data on disk.

	@author vl@fidra.de
*/
class CopySourceRandom : public CopySource
{
	public:
		CopySourceRandom(qint64 size, qint32 sectorsize);

	public:
		virtual bool open();
		virtual bool readSectors(void* buffer, qint64 readOffset, qint64 numSectors);
		virtual qint64 length() const;

		virtual qint32 sectorSize() const { return m_SectorSize; } /**< @return the file's sector size */
		virtual bool overlaps(const CopyTarget&) const { return false; } /**< @return false for random source */
		virtual qint64 firstSector() const { return 0; } /**< @return 0 for random source */
		virtual qint64 lastSector() const { return length(); } /**< @return equal to length for random source. @see length() */

	protected:
		QFile& random() { return m_Random; }
		const QFile& random() const { return m_Random; }
		qint32 size() const { return m_Size; }

	private:
		qint64 m_Size;
		qint32 m_SectorSize;
		QFile m_Random;
};

#endif
