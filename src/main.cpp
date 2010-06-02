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

#include "gui/mainwindow.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include "util/helpers.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <kaboutdata.h>

#include <config.h>

int main(int argc, char* argv[])
{
	KCmdLineArgs::init(argc, argv, createPartitionManagerAboutData());
	KCmdLineOptions options;
	options.add("dontsu", ki18nc("@info:shell", "Do not try to gain super user privileges"));
	options.add("advconfig", ki18nc("@info:shell", "Show advanced tab in configuration dialog"));
	options.add("+[device]", ki18nc("@info:shell", "Device(s) to manage"));
	KCmdLineArgs::addCmdLineOptions(options);

	// workaround for https://bugs.launchpad.net/kdesudo/+bug/272427
	unblockSigChild();

	KApplication app;

	registerMetaTypes();
	if (!checkPermissions())
		return 0;

	Config::instance("partitionmanagerrc");

	if (!loadBackend())
		return 0;

	MainWindow* mainWindow = new MainWindow();
	mainWindow->show();

	return app.exec();
}
