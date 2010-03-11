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

#include "gui/applyprogressdialog.h"

#include "gui/applyprogressdialogwidget.h"
#include "gui/applyprogressdetailswidget.h"

#include "core/operationrunner.h"

#include "ops/operation.h"

#include "jobs/job.h"

#include "util/report.h"

#include <QCloseEvent>
#include <QTime>
#include <QFont>
#include <QKeyEvent>
#include <QFile>

#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <krun.h>
#include <ktemporaryfile.h>
#include <kaboutdata.h>
#include <ktextedit.h>

const QString ApplyProgressDialog::m_TimeFormat = "hh:mm:ss";

static QWidget* mainWindow(QWidget* w)
{
	while (w && w->parentWidget())
		w = w->parentWidget();
	return w;
}

/** Creates a new ProgressDialog
	@param parent pointer to the parent widget
	@param orunner the OperationRunner whose progress this dialog is showing
*/
ApplyProgressDialog::ApplyProgressDialog(QWidget* parent, OperationRunner& orunner) :
	KDialog(parent),
	m_ProgressDialogWidget(new ApplyProgressDialogWidget(this)),
	m_ProgressDetailsWidget(new ApplyProgressDetailsWidget(this)),
	m_OperationRunner(orunner),
	m_Report(NULL),
	m_SavedParentTitle(mainWindow(this)->windowTitle()),
	m_Timer(this),
	m_Time(),
	m_CurrentOpItem(NULL),
	m_CurrentJobItem(NULL),
	m_LastReportUpdate(0)
{
	setMainWidget(&dialogWidget());
	setDetailsWidget(&detailsWidget());

	showButtonSeparator(true);
	setAttribute(Qt::WA_ShowModal, true);

	setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Details);

	dialogWidget().treeTasks().setColumnWidth(0, width() * 0.8);

	setupConnections();

	restoreDialogSize(KConfigGroup(KGlobal::config(), "applyProgressDialog"));
}

/** Destroys a ProgressDialog */
ApplyProgressDialog::~ApplyProgressDialog()
{
	KConfigGroup kcg(KGlobal::config(), "applyProgressDialog");
	saveDialogSize(kcg);
	delete m_Report;
}

void ApplyProgressDialog::setupConnections()
{
	connect(&operationRunner(), SIGNAL(progressSub(int)), &dialogWidget().progressSub(), SLOT(setValue(int)));
	connect(&operationRunner(), SIGNAL(finished()), SLOT(onAllOpsFinished()));
	connect(&operationRunner(), SIGNAL(cancelled()), SLOT(onAllOpsCancelled()));
	connect(&operationRunner(), SIGNAL(error()), SLOT(onAllOpsError()));
	connect(&operationRunner(), SIGNAL(opStarted(int, Operation*)), SLOT(onOpStarted(int, Operation*)));
	connect(&operationRunner(), SIGNAL(opFinished(int, Operation*)), SLOT(onOpFinished(int, Operation*)));
	connect(&timer(), SIGNAL(timeout()), SLOT(onSecondElapsed()));
	connect(&detailsWidget().buttonSave(), SIGNAL(clicked()), SLOT(saveReport()));
	connect(&detailsWidget().buttonBrowser(), SIGNAL(clicked()), SLOT(browserReport()));
}

/** Shows the dialog */
void ApplyProgressDialog::show()
{
	setStatus(i18nc("@info:progress", "Setting up..."));

	resetReport();

	dialogWidget().progressTotal().setRange(0, operationRunner().numJobs());
	dialogWidget().progressTotal().setValue(0);

	dialogWidget().treeTasks().clear();
	showButton(KDialog::Ok, false);
	showButton(KDialog::Cancel, true);

	timer().start(1000);
	time().start();

	setLastReportUpdate(0);

	onSecondElapsed(); // resets the total time output label

	KDialog::show();
}

void ApplyProgressDialog::resetReport()
{
	delete m_Report;
	m_Report = new Report(NULL);

	detailsWidget().editReport().clear();
	detailsWidget().editReport().setCursorWidth(0);
	detailsWidget().buttonSave().setEnabled(false);
	detailsWidget().buttonBrowser().setEnabled(false);

	connect(&report(), SIGNAL(outputChanged()), SLOT(updateReport()));
}

void ApplyProgressDialog::closeEvent(QCloseEvent* e)
{
	e->ignore();
	slotButtonClicked(operationRunner().isRunning() ? KDialog::Cancel : KDialog::Ok);
}

void ApplyProgressDialog::slotButtonClicked(int button)
{
	if (button == KDialog::Details)
	{
		KDialog::slotButtonClicked(button);
		updateReport(true);
		return;
	}

	if (button == KDialog::Cancel && operationRunner().isRunning())
	{
		// only cancel once
		if (operationRunner().isCancelling())
			return;

		KApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		enableButtonCancel(false);
		setStatus(i18nc("@info:progress", "Waiting for operation to finish..."));
		repaint();
		dialogWidget().repaint();

		// suspend the runner, so it doesn't happily carry on while the user decides
		// if he really wants to cancel
 		operationRunner().suspendMutex().lock();
		enableButtonCancel(true);

		KApplication::restoreOverrideCursor();

		if (KMessageBox::questionYesNo(this, i18nc("@info", "Do you really want to cancel?"), i18nc("@title:window", "Cancel Running Operations"), KGuiItem(i18nc("@action:button", "Yes, Cancel Operations")), KStandardGuiItem::no()) == KMessageBox::Yes)
			// in the meantime while we were showing the messagebox, the runner might have finished.
			if (operationRunner().isRunning())
				operationRunner().cancel();

		operationRunner().suspendMutex().unlock();

		return;
	}

	mainWindow(this)->setWindowTitle(savedParentTitle());

	KDialog::accept();
}

void ApplyProgressDialog::onAllOpsFinished()
{
	allOpsDone(i18nc("@info:progress", "All operations successfully finished."));
}

void ApplyProgressDialog::onAllOpsCancelled()
{
	allOpsDone(i18nc("@info:progress", "Operations cancelled."));
}

void ApplyProgressDialog::onAllOpsError()
{
	allOpsDone(i18nc("@info:progress", "There were errors while applying operations. Aborted."));
}

void ApplyProgressDialog::allOpsDone(const QString& msg)
{
	dialogWidget().progressTotal().setValue(operationRunner().numJobs());
	showButton(KDialog::Cancel, false);
	showButton(KDialog::Ok, true);
	detailsWidget().buttonSave().setEnabled(true);
	detailsWidget().buttonBrowser().setEnabled(true);
	timer().stop();
	updateReport(true);

	setStatus(msg);
}

void ApplyProgressDialog::updateReport(bool force)
{
	// Rendering the HTML in the KTextEdit is extremely expensive. So make sure not to do that
	// unnecessarily and not too often:
	// (1) If the widget isn't visible, don't update.
	// (2) Also don't update if the last update was n msecs ago, BUT
	// (3) DO update if we're being forced to.
	if (force || (detailsWidget().isVisible() && time().elapsed() - lastReportUpdate() > 2000))
	{
		detailsWidget().editReport().setHtml("<html><body>" + report().toHtml() + "</body></html>");
		detailsWidget().editReport().moveCursor(QTextCursor::End);
		detailsWidget().editReport().ensureCursorVisible();

		setLastReportUpdate(time().elapsed());
	}
}

void ApplyProgressDialog::onOpStarted(int num, Operation* op)
{
	addTaskOutput(num, *op);
	setStatus(op->description());

	dialogWidget().progressSub().setValue(0);
	dialogWidget().progressSub().setRange(0, op->totalProgress());

	connect(op, SIGNAL(jobStarted(Job*, Operation*)), SLOT(onJobStarted(Job*, Operation*)));
 	connect(op, SIGNAL(jobFinished(Job*, Operation*)), SLOT(onJobFinished(Job*, Operation*)));
}

void ApplyProgressDialog::onJobStarted(Job* job, Operation* op)
{
	for (qint32 i = 0; i < dialogWidget().treeTasks().topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = dialogWidget().treeTasks().topLevelItem(i);

		if (item == NULL || reinterpret_cast<const Operation*>(item->data(0, Qt::UserRole).toULongLong()) != op)
			continue;

		QTreeWidgetItem* child = new QTreeWidgetItem();
		child->setText(0, job->description());
		child->setIcon(0, job->statusIcon());
		child->setText(1, QTime(0, 0).toString(timeFormat()));
		item->addChild(child);
		dialogWidget().treeTasks().scrollToBottom();
		setCurrentJobItem(child);
		break;
	}
}

void ApplyProgressDialog::onJobFinished(Job* job, Operation* op)
{
	if (currentJobItem())
		currentJobItem()->setIcon(0, job->statusIcon());

	setCurrentJobItem(NULL);

	const int current = dialogWidget().progressTotal().value();
	dialogWidget().progressTotal().setValue(current + 1);

	setParentTitle(op->description());
	updateReport(true);
}

void ApplyProgressDialog::onOpFinished(int num, Operation* op)
{
	if (currentOpItem())
	{
		currentOpItem()->setText(0, opDesc(num, *op));
		currentOpItem()->setIcon(0, op->statusIcon());
	}

	setCurrentOpItem(NULL);

	setStatus(op->description());

	dialogWidget().progressSub().setValue(op->totalProgress());
	updateReport(true);
}

void ApplyProgressDialog::setParentTitle(const QString& s)
{
	const int percent = dialogWidget().progressTotal().value() * 100 / dialogWidget().progressTotal().maximum();
	mainWindow(this)->setWindowTitle(QString::number(percent) + "% - " + s + " - " + savedParentTitle());
}

void ApplyProgressDialog::setStatus(const QString& s)
{
	setCaption(s);
	dialogWidget().status().setText(s);

	setParentTitle(s);
}

QString ApplyProgressDialog::opDesc(int num, const Operation& op) const
{
	return i18nc("@info:progress", "[%1/%2] - %3: %4", num, operationRunner().numOperations(), op.statusText(), op.description());
}

void ApplyProgressDialog::addTaskOutput(int num, const Operation& op)
{
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setIcon(0, op.statusIcon());
	item->setText(0, opDesc(num, op));
	item->setText(1, QTime(0, 0).toString(timeFormat()));

	QFont f;
	f.setWeight(QFont::Bold);
	item->setFont(0, f);
	item->setFont(1, f);

	item->setData(0, Qt::UserRole, reinterpret_cast<const qulonglong>(&op));
	dialogWidget().treeTasks().addTopLevelItem(item);
	dialogWidget().treeTasks().scrollToBottom();
	setCurrentOpItem(item);
}

void ApplyProgressDialog::onSecondElapsed()
{
	if (currentJobItem())
	{
		QTime t = QTime::fromString(currentJobItem()->text(1), timeFormat()).addSecs(1);
		currentJobItem()->setText(1, t.toString(timeFormat()));
	}

	if (currentOpItem())
	{
		QTime t = QTime::fromString(currentOpItem()->text(1), timeFormat()).addSecs(1);
		currentOpItem()->setText(1, t.toString(timeFormat()));;
	}

	const QTime outputTime = QTime(0, 0).addMSecs(time().elapsed());
	dialogWidget().totalTime().setText(i18nc("@info:progress", "Total Time: %1", outputTime.toString(timeFormat())));
}

void ApplyProgressDialog::keyPressEvent(QKeyEvent* e)
{
	e->accept();

	switch(e->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
			if (isButtonEnabled(KDialog::Ok))
				slotButtonClicked(KDialog::Ok);
			break;

		case Qt::Key_Escape:
			slotButtonClicked(isButtonEnabled(KDialog::Cancel) ? KDialog::Cancel : KDialog::Ok);
			break;

		default:
			break;
	}
}

void ApplyProgressDialog::saveReport()
{
	QString fileName = KFileDialog::getSaveFileName(KUrl("kfiledialog://saveReport"));

	if (fileName.isEmpty())
		return;

	if (!QFile::exists(fileName) || KMessageBox::warningContinueCancel(this, i18nc("@info", "Do you want to overwrite the existing file <filename>%1</filename>?", fileName), i18nc("@title:window", "Overwrite Existing File?"), KGuiItem(i18nc("@action:button", "Overwrite File")), KStandardGuiItem::cancel()) == KMessageBox::Continue)
	{
		QFile file(fileName);

		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			file.write(Report::htmlHeader().toUtf8());
			file.write(report().toHtml().toUtf8());
			file.write(Report::htmlFooter().toUtf8());

			file.close();
		}
		else
			KMessageBox::sorry(this, i18nc("@info", "Could not open file <filename>%1</filename> for writing.", fileName), i18nc("@title:window", "Could Not Save Report."));
	}
}

void ApplyProgressDialog::browserReport()
{
	KTemporaryFile file;

	// Make sure the temp file is created somewhere another user can read it: KRun::runUrl() will open
	// the file as the logged in user, not as the user running our application.
	file.setFileTemplate("/tmp/" + KGlobal::mainComponent().aboutData()->appName() + "-XXXXXX.html");
	file.setAutoRemove(false);

	if (file.open())
	{
		file.write(Report::htmlHeader().toUtf8());
		file.write(report().toHtml().toUtf8());
		file.write(Report::htmlFooter().toUtf8());

		// set the temp file's permission for everyone to read it.
		file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);

		if (!KRun::runUrl(file.fileName(), "text/html", this, true))
			KMessageBox::sorry(this, i18nc("@info", "The configured external browser could not be run. Please check your settings."), i18nc("@title:window", "Could Not Launch Browser."));
	}
	else
		KMessageBox::sorry(this, i18nc("@info", "Could not create temporary file <filename>%1</filename> for writing.", file.fileName()), i18nc("@title:window", "Could Not Launch Browser."));
}
