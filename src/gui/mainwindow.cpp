/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
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
#include "gui/infopane.h"
#include "gui/progressdialog.h"

#include "core/device.h"

#include <kstandardaction.h>
#include <kactioncollection.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kstandardguiitem.h>

#include <QCloseEvent>

#include <config.h>

/** Creates a new MainWindow instance.
	@param parent the parent widget
	@param coll an action collection if used as a KPart
*/
MainWindow::MainWindow(QWidget* parent, KActionCollection* coll) :
	KXmlGuiWindow(parent),
	Ui::MainWindowBase(),
	m_StatusText(new QLabel(this)),
	m_InfoPane(new InfoPane(this)),
	m_ActionCollection(coll)
{
	setupUi(this);
	init();
}

void MainWindow::init()
{
	treeLog().init(actionCollection(), &pmWidget());

	connect(GlobalLog::instance(), SIGNAL(newMessage(Log::Level, const QString&)), &treeLog(), SLOT(onNewLogMessage(Log::Level, const QString&)));

	setupActions();
	setupStatusBar();
	setupConnections();

	listDevices().init(actionCollection(), &pmWidget());
	listOperations().init(actionCollection(), &pmWidget());
	pmWidget().init(actionCollection(), "partitionmanagerrc");

	// If we were called with an action collection we're supposed to be a KPart, so don't
	// create the GUI in that case.
	if (m_ActionCollection != NULL)
		setupGUI(ToolBar | Keys | StatusBar | Save);
	else
		setupGUI(ToolBar | Keys | StatusBar | Save | Create);

	loadConfig();

	dockInformation().setWidget(&infoPane());

	// trigger an update for the info pane so it can re-layout itself
	updateSelection(NULL);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (pmWidget().progressDialog().isVisible())
	{
		event->ignore();
		return;
	}

	if (pmWidget().numPendingOperations() > 0)
	{
		if (KMessageBox::warningContinueCancel(this,
			i18ncp("@info", "<para>Do you really want to quit the application?</para><para>There is still an operation pending.</para>",
    		"<para>Do you really want to quit the application?</para><para>There are still %1 operations pending.</para>", pmWidget().numPendingOperations()),
			i18nc("@title:window", "Discard Pending Operations and Quit?"),
			KGuiItem(i18nc("@action:button", "&Quit <application>%1</application>", KGlobal::mainComponent().aboutData()->programName())),
			KStandardGuiItem::cancel(), "reallyQuit") == KMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}

	saveConfig();

	KXmlGuiWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent* event)
{
	if ((event->type() == QEvent::ActivationChange || event->type() == QEvent::WindowStateChange) && event->spontaneous() && isActiveWindow() && pmWidget().progressDialog().isVisible())
	{
		pmWidget().progressDialog().activateWindow();
		pmWidget().progressDialog().raise();
	}

	KXmlGuiWindow::changeEvent(event);
}

void MainWindow::setupActions()
{
	// File actions
	KStandardAction::quit(this, SLOT(close()), actionCollection());

	// View actions
	actionCollection()->addAction("toggleDockDevices", dockDevices().toggleViewAction());
	actionCollection()->addAction("toggleDockOperations", dockOperations().toggleViewAction());
	actionCollection()->addAction("toggleDockInformation", dockInformation().toggleViewAction());
	actionCollection()->addAction("toggleDockLog", dockLog().toggleViewAction());
}

void MainWindow::setupConnections()
{
	connect(&pmWidget(), SIGNAL(devicesChanged()), SLOT(updateDevices()));
	connect(&pmWidget(), SIGNAL(operationsChanged()), &listOperations(), SLOT(updateOperations()));
	connect(&pmWidget(), SIGNAL(statusChanged()), SLOT(updateStatusBar()));
	connect(&pmWidget(), SIGNAL(selectionChanged(const Partition*)), SLOT(updateSelection(const Partition*)));
	connect(&dockInformation(), SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(onDockLocationChanged(Qt::DockWidgetArea)));
}

void MainWindow::setupStatusBar()
{
	statusBar()->addWidget(&statusText());
	updateStatusBar();
}

void MainWindow::loadConfig()
{
	if (Config::firstRun())
	{
		dockLog().setVisible(false);
		dockInformation().setVisible(false);
		toolBar("deviceToolBar")->setVisible(false);
	}
}

void MainWindow::saveConfig() const
{
	Config::setFirstRun(false);
	Config::self()->writeConfig();
}

void MainWindow::updateStatusBar()
{
	statusText().setText(i18ncp("@info:status", "One pending operation", "%1 pending operations", pmWidget().numPendingOperations()));
}

void MainWindow::updateDevices()
{
	listDevices().updateDevices();

	if (pmWidget().selectedDevice())
		infoPane().showDevice(dockWidgetArea(&dockInformation()), *pmWidget().selectedDevice());
	else
		infoPane().clear();

	updateWindowTitle();
}

void MainWindow::on_m_ListDevices_selectionChanged(Device* d)
{
	pmWidget().setSelectedDevice(d);
	updateSelection(NULL);
}

void MainWindow::onDockLocationChanged(Qt::DockWidgetArea)
{
	updateSelection(pmWidget().selectedPartition());
}

void MainWindow::updateWindowTitle()
{
	QString title;

	if (pmWidget().selectedDevice())
		title = pmWidget().selectedDevice()->deviceNode() + " - ";

	title += KGlobal::mainComponent().aboutData()->programName() + ' ' + KGlobal::mainComponent().aboutData()->version();

	setWindowTitle(title);
}

void MainWindow::updateSelection(const Partition* p)
{
	if (p)
		infoPane().showPartition(dockWidgetArea(&dockInformation()), *p);
	else if (pmWidget().selectedDevice())
		infoPane().showDevice(dockWidgetArea(&dockInformation()), *pmWidget().selectedDevice());
	else
		infoPane().clear();

	updateWindowTitle();
}

