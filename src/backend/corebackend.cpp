/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de                        *
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

#include "backend/corebackend.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>

static const char pluginName[] = "pluginpmlibparted";
// static const char pluginName[] = "pluginpmdummy";

CoreBackend* CoreBackend::self()
{
	// This could be used to load any kind of backend if there were more than one
	// to choose from. So right now it's just loading the parted plugin and returning
	// it.
	static CoreBackend* instance = NULL;

	if (instance == NULL)
	{
		KPluginLoader loader(pluginName);

		KPluginFactory* factory = loader.factory();

		if (factory != NULL)
			instance = factory->create<CoreBackend>(NULL);
		else
			kWarning() << "Could not load instance plugin for core backend: " << loader.errorString();
	}

	return instance;
}

void CoreBackend::emitProgress(int i)
{
	emit progress(i);
}

void CoreBackend::setPartitionTableForDevice(Device& d, PartitionTable* p)
{
	d.setPartitionTable(p);
}

void CoreBackend::setPartitionTableMaxPrimaries(PartitionTable& p, qint32 max_primaries)
{
	p.setMaxPrimaries(max_primaries);
}

