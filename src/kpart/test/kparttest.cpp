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

#include "kparttest.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <kdebug.h>

#include <QApplication>

KPartTest::KPartTest() :
	KParts::MainWindow(),
	m_Part(NULL)
{
	setupActions();

	KPluginFactory* factory = KPluginLoader("partitionmanagerpart").factory();
	m_Part = factory ? factory->create<KParts::ReadOnlyPart>("PartitionManagerPart", this) : NULL;

	if (m_Part == NULL)
	{
		KMessageBox::error(this, "Could not load Partition Manager's KPart");
		qApp->quit();
		return;
	}

	setCentralWidget(m_Part->widget());

	setupGUI(ToolBar | Keys | StatusBar | Save);
	createGUI(m_Part);
}

void KPartTest::setupActions()
{
	KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());
}
