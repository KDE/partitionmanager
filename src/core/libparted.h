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

#if !defined(LIBPARTED__H)

#define LIBPARTED__H

#include <qglobal.h>

class OperationStack;
class Device;

class QString;

/** @brief Scanning for Devices.

	Encapsulates Device scanning done by libparted.

	More libparted-related stuff is in the @link Job Job-derived @endlink classes.

	@author vl@fidra.de
*/
class LibParted
{
	Q_DISABLE_COPY(LibParted)

	public:
		LibParted();

	public:
		void scanDevices(OperationStack& ostack);
		static quint64 firstUsableSector(const Device& d);
		static quint64 lastUsableSector(const Device& d);
};

#endif
