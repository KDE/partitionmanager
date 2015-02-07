/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#include "backend/corebackend.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include "util/globallog.h"

#include <QDebug>

#include <config.h>

class CoreBackend::CoreBackendPrivate
{
	public:
		CoreBackendPrivate() {}
};

CoreBackend::CoreBackend() :
	d(new CoreBackendPrivate())
{
}

CoreBackend::~CoreBackend()
{
	delete d;
}

void CoreBackend::emitProgress(int i)
{
	emit progress(i);
}

void CoreBackend::emitScanProgress(const QString& device_node, int i)
{
	emit scanProgress(device_node, i);
}

void CoreBackend::setPartitionTableForDevice(Device& d, PartitionTable* p)
{
	d.setPartitionTable(p);
}

void CoreBackend::setPartitionTableMaxPrimaries(PartitionTable& p, qint32 max_primaries)
{
	p.setMaxPrimaries(max_primaries);
}
