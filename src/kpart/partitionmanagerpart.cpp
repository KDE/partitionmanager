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

#include "kpart/partitionmanagerpart.h"

#include "gui/mainwindow.h"

#include "util/helpers.h"

#include <kpluginfactory.h>

K_PLUGIN_FACTORY(PartitionManagerPartFactory, registerPlugin<PartitionManagerPart>("PartitionManagerPart");)
K_EXPORT_PLUGIN(PartitionManagerPartFactory("partitionmanagerpart", "partitionmanager"))

PartitionManagerPart::PartitionManagerPart(QWidget*, QObject* parent, const QVariantList&) :
	KParts::ReadOnlyPart(parent)
{
	setComponentData(PartitionManagerPartFactory::componentData(), false);

	// workaround for https://bugs.launchpad.net/kdesudo/+bug/272427
	unblockSigChild();

	registerMetaTypes();

	setWidget(new MainWindow(NULL, actionCollection()));
	setXMLFile("partitionmanagerpart.rc");
}

KAboutData* PartitionManagerPart::createAboutData()
{
	return createPartitionManagerAboutData();
}

