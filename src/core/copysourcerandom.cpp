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

#include "core/copysourcerandom.h"

#include <kdebug.h>

/** Constructs a CopySourceRandom with the given @p size
	@param s the size the copy source will (pretend to) have
	@param sectorsize the sectorsize the copy source will (pretend to) have
*/
CopySourceRandom::CopySourceRandom (qint64 s, qint32 sectorsize) :
	CopySource(),
	m_Size(s),
	m_SectorSize(sectorsize),
	m_Random("/dev/urandom")
{
}

/** Opens the random source.
	@return true on success
*/
bool CopySourceRandom::open()
{
	return random().open(QIODevice::ReadOnly);
}

/** Returns the length of the random source in sectors.
	@return length of the source in sectors.
*/
qint64 CopySourceRandom::length() const
{
	return size() / sectorSize();
}

/** Reads the given number of sectors from the random source into the given buffer.
	@param buffer buffer to store the sectors read in
	@param readOffset offset where to begin reading (unused)
	@param numSectors number of sectors to read
	@return true on success
*/
bool CopySourceRandom::readSectors(void* buffer, qint64 readOffset, qint64 numSectors)
{
	Q_UNUSED(readOffset);

	return random().read(static_cast<char*>(buffer), numSectors * sectorSize()) == numSectors * sectorSize();
}
