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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#if !defined(PARTITIONMANAGERPART__H)

#define PARTITIONMANAGERPART__H

#include <kparts/part.h>

class KAboutData;

class PartitionManagerPart : public KParts::ReadOnlyPart
{
	Q_OBJECT

	public:
		PartitionManagerPart(QWidget* parentWidget, QObject* parent, const QVariantList& args);

	public:
		static KAboutData* createAboutData();

	protected:
	    virtual bool openFile() { return false; }
};

#endif
