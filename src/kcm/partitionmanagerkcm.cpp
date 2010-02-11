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

#include <config.h>

#include <kgenericfactory.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <ktoolbar.h>
#include <kapplication.h>
#include <kcmultidialog.h>

#include <QTimer>

K_PLUGIN_FACTORY(
		PartitionManagerKCMFactory,
		registerPlugin<PartitionManagerKCM>();
)
K_EXPORT_PLUGIN(
		PartitionManagerKCMFactory("kcm_partitionmanager", "partitionmanager")
)

PartitionManagerKCM::PartitionManagerKCM(QWidget* parent, const QVariantList&) :
	KCModule(PartitionManagerKCMFactory::componentData(), parent),
	Ui::PartitionManagerKCMBase(),
	m_ActionCollection(new KActionCollection(this, PartitionManagerKCMFactory::componentData()))
{
	setupUi(this);

	connect(GlobalLog::instance(), SIGNAL(newMessage(Log::Level, const QString&)), SLOT(onNewLogMessage(Log::Level, const QString&)));

	// workaround for https://bugs.launchpad.net/kdesudo/+bug/272427
	unblockSigChild();
	registerMetaTypes();

	setButtons(Apply);
	setupConnections();

	listDevices().init(actionCollection(), &pmWidget());
	listOperations().init(actionCollection(), &pmWidget());
	pmWidget().init(actionCollection(), "kcm_partitionmanagerrc");

	const char* actionNames[] =
	{
		"newPartition",
		"resizePartition",
		"deletePartition",
		"copyPartition",
		"pastePartition",
		"checkPartition",
		"propertiesPartition",
		"backupPartition",
		"restorePartition",
		"",
		"createNewPartitionTable",
		"refreshDevices"
	};

	for (size_t i = 0; i < sizeof(actionNames) / sizeof(actionNames[0]); i++)
		if (strlen(actionNames[i]) > 0)
			toolBar().addAction(actionCollection()->action(actionNames[i]));
		else
			toolBar().addSeparator();

	toolBar().setIconSize(QSize(22, 22));
	toolBar().setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	splitterHorizontal().setStretchFactor(0, 1);
	splitterHorizontal().setStretchFactor(1, 4);
	splitterVertical().setStretchFactor(0, 1);
	splitterVertical().setStretchFactor(1, 3);

	setupKCMWorkaround();
	setAboutData(createPartitionManagerAboutData());
}

void PartitionManagerKCM::onNewLogMessage(Log::Level, const QString& s)
{
	kDebug() << s;
}

void PartitionManagerKCM::setupConnections()
{
	connect(&pmWidget(), SIGNAL(devicesChanged()), &listDevices(), SLOT(updateDevices()));
	connect(&pmWidget(), SIGNAL(operationsChanged()), &listOperations(), SLOT(updateOperations()));
	connect(&listDevices(), SIGNAL(selectionChanged(Device*)), &pmWidget(), SLOT(setSelectedDevice(Device*)));
	connect(&pmWidget(), SIGNAL(operationsChanged()), SLOT(onStatusChanged()));
}

void PartitionManagerKCM::onStatusChanged()
{
	emit changed(pmWidget().numPendingOperations() > 0);
}

void PartitionManagerKCM::setupKCMWorkaround()
{
	// The Partition Manager kcm must be run as root, for obvious reasons. system-settings will
	// open kcms that require root privileges in a separate kcmshell4 process with a window of
	// its own. This window (a KDialog, actually) has a couple of buttons at the bottom, one of
	// them an Ok-button. The user will expect to have his changes applied if he clicks that button.
	// Unfortunately, we cannot do that: The kcmshell will kill us and our OperationRunner thread
	// without asking us as soon as we return from PartitionManagerKCM::save(). Even worse, we
	// have no way to find out if PartitionMangerKCM::save() was called because the user clicked
	// on "Ok" or "Apply" -- if we had that way we could at least do nothing in the case of the
	// Ok button...
	// Anyway, there seems to be no other solution than find the KDialog and turn off all buttons we
	// cannot handle... Nasty, but effective for now.
	foreach(QWidget* w, KApplication::topLevelWidgets())
	{
		KCMultiDialog* dlg = qobject_cast<KCMultiDialog*>(w);
		if (dlg != NULL)
		{
			dlg->setButtons(KDialog::Cancel|KDialog::Apply);
			dlg->enableButtonApply(false);
			connect(dlg, SIGNAL(applyClicked()), SLOT(onApplyClicked()));
		}
	}
}

void PartitionManagerKCM::onApplyClicked()
{
	if (pmWidget().numPendingOperations() > 0)
		actionCollection()->action("applyAllOperations")->trigger();

	QTimer::singleShot(0, this, SLOT(onStatusChanged()));
}

