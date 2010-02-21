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
#include "gui/applyprogressdialog.h"
#include "gui/scanprogressdialog.h"

#include "core/device.h"

#include <kstandardaction.h>
#include <kactioncollection.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kstandardguiitem.h>

#include <QCloseEvent>
#include <QReadLocker>

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

	listDevices().setActionCollection(actionCollection());
	listOperations().setActionCollection(actionCollection());
	pmWidget().init(actionCollection(), "partitionmanagerrc");

	if (isKPart())
		setupGUI(ToolBar | Keys | StatusBar | Save);
	else
		setupGUI(ToolBar | Keys | StatusBar | Save | Create);

	loadConfig();

	dockInformation().setWidget(&infoPane());

	infoPane().clear();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (pmWidget().applyProgressDialog().isVisible())
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
	if ((event->type() == QEvent::ActivationChange || event->type() == QEvent::WindowStateChange) && event->spontaneous() && isActiveWindow())
	{
		QWidget* w = NULL;

		if (pmWidget().applyProgressDialog().isVisible())
			w = &pmWidget().applyProgressDialog();
		else if (pmWidget().scanProgressDialog().isVisible())
			w = &pmWidget().scanProgressDialog();

		if (w != NULL)
		{
			w->activateWindow();
			w->raise();
			w->setFocus();
		}
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

	// Settings Actions
	KStandardAction::showMenubar(this, SLOT(onShowMenuBar()), actionCollection());
}

void MainWindow::setupConnections()
{
	connect(&listDevices(), SIGNAL(selectionChanged(const QString&)), &pmWidget(), SLOT(setSelectedDevice(const QString&)));
	connect(&listDevices(), SIGNAL(deviceDoubleClicked(const QString&)), &pmWidget(), SLOT(onPropertiesDevice(const QString&)));
}

void MainWindow::setupStatusBar()
{
	statusBar()->addWidget(&statusText());
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

void MainWindow::on_m_PartitionManagerWidget_operationsChanged()
{
	listOperations().updateOperations(pmWidget().operations());

	statusText().setText(i18ncp("@info:status", "One pending operation", "%1 pending operations", pmWidget().numPendingOperations()));
}

void MainWindow::on_m_PartitionManagerWidget_devicesChanged()
{
	QReadLocker lockDevices(&pmWidget().operationStack().lock());

	listDevices().updateDevices(pmWidget().previewDevices());

	if (pmWidget().selectedDevice())
		infoPane().showDevice(dockWidgetArea(&dockInformation()), *pmWidget().selectedDevice());
	else
		infoPane().clear();

	updateWindowTitle();
}

void MainWindow::on_m_DockInformation_dockLocationChanged(Qt::DockWidgetArea)
{
	on_m_PartitionManagerWidget_selectedPartitionChanged(pmWidget().selectedPartition());
}

void MainWindow::updateWindowTitle()
{
	QString title;

	if (pmWidget().selectedDevice())
		title = pmWidget().selectedDevice()->deviceNode() + " - ";

	title += KGlobal::mainComponent().aboutData()->programName() + ' ' + KGlobal::mainComponent().aboutData()->version();

	setWindowTitle(title);
}

void MainWindow::on_m_PartitionManagerWidget_selectedPartitionChanged(const Partition* p)
{
	if (p)
		infoPane().showPartition(dockWidgetArea(&dockInformation()), *p);
	else if (pmWidget().selectedDevice())
		infoPane().showDevice(dockWidgetArea(&dockInformation()), *pmWidget().selectedDevice());
	else
		infoPane().clear();

	updateWindowTitle();
}

bool MainWindow::isKPart() const
{
	// If we were instantiated with an action collection we're supposed to be a KPart
	return m_ActionCollection != NULL;
}

void MainWindow::onShowMenuBar()
{
	QAction* menuBarAction = actionCollection()->action(KStandardAction::name(KStandardAction::ShowMenubar));
	if (menuBarAction->isChecked())
		menuBar()->show();
	else
	{
		const QString accel = menuBarAction->shortcut().toString();
		KMessageBox::information(this, i18nc("@info", "This will hide the menu bar completely. You can show it again by typing %1.", accel), i18nc("@window:title", "Hide Menu Bar"), "hideMenuBarWarning");

		menuBar()->hide();
	}

	Config::self()->setShowMenuBar(menuBarAction->isChecked());
}
