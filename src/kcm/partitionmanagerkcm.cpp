/***************************************************************************
 *   Copyright (C) 2009 by Volker Lanz <vl@fidra.de>                       *
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

#include "kcm/partitionmanagerkcm.h"

#include "gui/partitionmanagerwidget.h"
#include "gui/listdevices.h"

#include "util/helpers.h"

#include <kgenericfactory.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <ktoolbar.h>

K_PLUGIN_FACTORY(
		PartitionManagerKCMFactory,
		registerPlugin<PartitionManagerKCM>();
)
K_EXPORT_PLUGIN(
		PartitionManagerKCMFactory("partitionmanagerkcm")
)

PartitionManagerKCM::PartitionManagerKCM(QWidget* parent, const QVariantList&) :
	KCModule(PartitionManagerKCMFactory::componentData(), parent),
	Ui::PartitionManagerKCMBase(),
	m_ActionCollection(new KActionCollection(this, PartitionManagerKCMFactory::componentData()))
{
	setupUi(this);

	connect(GlobalLog::instance(), SIGNAL(newMessage(log::Level, const QString&)), SLOT(onNewLogMessage(log::Level, const QString&)));

	// workaround for https://bugs.launchpad.net/kdesudo/+bug/272427
	unblockSigChild();
	registerMetaTypes();

	setButtons(Apply);

	setupConnections();

	listDevices().init(actionCollection(), &pmWidget());
	listOperations().init(actionCollection(), &pmWidget());
	pmWidget().init(actionCollection());

	const char* actionNames[] =
	{
		"applyAllOperations",
		"undoOperation",
		"clearAllOperations",
		"",
// 		"refreshDevices",
		"createNewPartitionTable",
		"",
		"newPartition",
		"resizePartition",
		"deletePartition",
		"copyPartition",
		"pastePartition",
// 		"mountPartition",
		"checkPartition",
		"propertiesPartition",
		"backupPartition",
		"restorePartition",
// 		"",
// 		"fileSystemSupport"
	};

	for(size_t i = 0; i < sizeof(actionNames) / sizeof(actionNames[0]); i++)
		if (strlen(actionNames[i]) > 0)
			toolBar()->addAction(actionCollection()->action(actionNames[i]));
		else
			toolBar()->addSeparator();

	toolBar()->setIconSize(QSize(22, 22));
	toolBar()->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

void PartitionManagerKCM::onNewLogMessage(log::Level, const QString& s)
{
	kDebug() << s;
}

void PartitionManagerKCM::load()
{
}

void PartitionManagerKCM::save()
{
}

void PartitionManagerKCM::setupConnections()
{
	connect(&pmWidget(), SIGNAL(devicesChanged()), &listDevices(), SLOT(updateDevices()));
	connect(&pmWidget(), SIGNAL(operationsChanged()), &listOperations(), SLOT(updateOperations()));
}

void PartitionManagerKCM::on_m_ListDevices_selectionChanged(Device* d)
{
	pmWidget().setSelectedDevice(d);
}
