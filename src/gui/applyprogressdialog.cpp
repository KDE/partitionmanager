/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/applyprogressdialog.h"

#include "gui/applyprogressdialogwidget.h"
#include "gui/applyprogressdetailswidget.h"

#include <core/operationrunner.h>

#include <ops/operation.h>

#include <jobs/job.h>

#include <util/report.h>
#include <util/htmlreport.h>

#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QFont>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QPushButton>
#include <QTemporaryFile>
#include <QTextStream>

#include <KAboutData>
#include <KConfigGroup>
#include <KIO/CopyJob>
#include <KIO/OpenUrlJob>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

const QString ApplyProgressDialog::m_TimeFormat = QStringLiteral("hh:mm:ss");

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
    QDialog(parent),
    m_ProgressDialogWidget(new ApplyProgressDialogWidget(this)),
    m_ProgressDetailsWidget(new ApplyProgressDetailsWidget(this)),
    m_OperationRunner(orunner),
    m_Report(nullptr),
    m_SavedParentTitle(mainWindow(this)->windowTitle()),
    m_Timer(this),
    m_ElapsedTimer(),
    m_CurrentOpItem(nullptr),
    m_CurrentJobItem(nullptr),
    m_LastReportUpdate(0)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&dialogWidget());
    QFrame* detailsBox = new QFrame(this);
    mainLayout->addWidget(detailsBox);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsBox);
    detailsLayout->addWidget(&detailsWidget());
    detailsWidget().hide();

    setAttribute(Qt::WA_ShowModal, true);

    dialogButtonBox = new QDialogButtonBox;
    okButton = dialogButtonBox->addButton(QDialogButtonBox::Ok);
    cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
    detailsButton = new QPushButton;
    detailsButton->setText(xi18nc("@action:button", "&Details") + QStringLiteral(" >>"));
    detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    dialogButtonBox->addButton(detailsButton, QDialogButtonBox::ActionRole);
    mainLayout->addWidget(dialogButtonBox);

    dialogWidget().treeTasks().setColumnWidth(0, width() * 8 / 10);
    detailsWidget().buttonBrowser().setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    detailsWidget().buttonSave().setIcon(QIcon::fromTheme(QStringLiteral("document-save")));

    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("applyProgressDialog"));
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a ProgressDialog */
ApplyProgressDialog::~ApplyProgressDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("applyProgressDialog"));
    kcg.writeEntry("Geometry", saveGeometry());
    delete m_Report;
}

void ApplyProgressDialog::setupConnections()
{
    connect(&operationRunner(), &OperationRunner::progressSub, &dialogWidget().progressSub(), &QProgressBar::setValue);
    connect(&operationRunner(), &OperationRunner::finished, this, &ApplyProgressDialog::onAllOpsFinished);
    connect(&operationRunner(), &OperationRunner::cancelled, this, &ApplyProgressDialog::onAllOpsCancelled);
    connect(&operationRunner(), &OperationRunner::error, this, &ApplyProgressDialog::onAllOpsError);
    connect(&operationRunner(), &OperationRunner::opStarted, this, &ApplyProgressDialog::onOpStarted);
    connect(&operationRunner(), &OperationRunner::opFinished, this, &ApplyProgressDialog::onOpFinished);
    connect(&timer(), &QTimer::timeout, this, &ApplyProgressDialog::onSecondElapsed);
    connect(&detailsWidget().buttonSave(), &QPushButton::clicked, this, &ApplyProgressDialog::saveReport);
    connect(&detailsWidget().buttonBrowser(), &QPushButton::clicked, this, &ApplyProgressDialog::browserReport);
    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &ApplyProgressDialog::onOkButton);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &ApplyProgressDialog::onCancelButton);
    connect(detailsButton, &QPushButton::clicked, this, &ApplyProgressDialog::toggleDetails);
}

/** Shows the dialog */
void ApplyProgressDialog::show()
{
    setStatus(xi18nc("@info:progress", "Setting up..."));

    resetReport();

    dialogWidget().progressTotal().setRange(0, operationRunner().numJobs());
    dialogWidget().progressTotal().setValue(0);

    dialogWidget().treeTasks().clear();
    okButton->setVisible(false);
    okButton->setEnabled(false);
    cancelButton->setVisible(true);
    cancelButton->setEnabled(true);


    timer().start(1000);
    time().start();

    setLastReportUpdate(0);

    onSecondElapsed(); // resets the total time output label

    QDialog::show();
}

void ApplyProgressDialog::resetReport()
{
    delete m_Report;
    m_Report = new Report(nullptr);

    detailsWidget().editReport().clear();
    detailsWidget().editReport().setCursorWidth(0);
    detailsWidget().buttonSave().setEnabled(false);
    detailsWidget().buttonBrowser().setEnabled(false);

    connect(&report(), &Report::outputChanged, this, &ApplyProgressDialog::updateReportUnforced);
}

void ApplyProgressDialog::closeEvent(QCloseEvent* e)
{
    e->ignore();
    operationRunner().isRunning() ? onCancelButton() : onOkButton();
}

void ApplyProgressDialog::toggleDetails()
{
    const bool isVisible = detailsWidget().isVisible();
    detailsWidget().setVisible(!isVisible);
    detailsButton->setText(xi18nc("@action:button", "&Details") + (isVisible ? QStringLiteral(" >>") : QStringLiteral(" <<")));
}

void ApplyProgressDialog::onDetailsButton()
{
    updateReport(true);
    return;
}

void ApplyProgressDialog::onCancelButton()
{
    if (operationRunner().isRunning()) {
        // only cancel once
        if (operationRunner().isCancelling())
            return;

        // suspend the runner, so it doesn't happily carry on while the user decides
        // if he really wants to cancel
        operationRunner().suspendMutex().lock();
        if (KMessageBox::warningTwoActions(this, xi18nc("@info", "Do you really want to cancel?"), xi18nc("@title:window", "Cancel Running Operations"), KGuiItem(xi18nc("@action:button", "Yes, Cancel Operations"), QStringLiteral("dialog-ok")), KStandardGuiItem::cancel()) == KMessageBox::PrimaryAction)
            // in the meantime while we were showing the messagebox, the runner might have finished.
            if (operationRunner().isRunning()) {
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                cancelButton->setEnabled(false);
                setStatus(xi18nc("@info:progress", "Waiting for operation to finish..."));
                repaint();
                dialogWidget().repaint();
                QApplication::restoreOverrideCursor();
                operationRunner().cancel();
            }

        operationRunner().suspendMutex().unlock();
    }
    return;
}

void ApplyProgressDialog::onOkButton()
{
    mainWindow(this)->setWindowTitle(savedParentTitle());

    QDialog::accept();
}

void ApplyProgressDialog::onAllOpsFinished()
{
    allOpsDone(xi18nc("@info:progress", "All operations successfully finished."));
}

void ApplyProgressDialog::onAllOpsCancelled()
{
    allOpsDone(xi18nc("@info:progress", "Operations cancelled."));
}

void ApplyProgressDialog::onAllOpsError()
{
    allOpsDone(xi18nc("@info:progress", "There were errors while applying operations. Aborted."));
}

void ApplyProgressDialog::allOpsDone(const QString& msg)
{
    dialogWidget().progressTotal().setValue(operationRunner().numJobs());
    cancelButton->setVisible(false);
    okButton->setVisible(true);
    okButton->setEnabled(true);
    detailsWidget().buttonSave().setEnabled(true);
    detailsWidget().buttonBrowser().setEnabled(true);
    timer().stop();
    updateReport(true);

    setStatus(msg);
}

void ApplyProgressDialog::updateReportUnforced()
{
    updateReport(false);
}

void ApplyProgressDialog::updateReport(bool force)
{
    // Rendering the HTML in the QTextEdit is extremely expensive. So make sure not to do that
    // unnecessarily and not too often:
    // (1) If the widget isn't visible, don't update.
    // (2) Also don't update if the last update was n msecs ago, BUT
    // (3) DO update if we're being forced to.
    if (force || (detailsWidget().isVisible() && time().elapsed() - lastReportUpdate() > 2000)) {
        detailsWidget().editReport().setHtml(QStringLiteral("<html><body>") + report().toHtml() + QStringLiteral("</body></html>"));
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

    connect(op, &Operation::jobStarted, this, &ApplyProgressDialog::onJobStarted);
    connect(op, &Operation::jobFinished, this, &ApplyProgressDialog::onJobFinished);
}

void ApplyProgressDialog::onJobStarted(Job* job, Operation* op)
{
    for (qint32 i = 0; i < dialogWidget().treeTasks().topLevelItemCount(); i++) {
        QTreeWidgetItem* item = dialogWidget().treeTasks().topLevelItem(i);

        if (item == nullptr || reinterpret_cast<const Operation*>(item->data(0, Qt::UserRole).toULongLong()) != op)
            continue;

        QTreeWidgetItem* child = new QTreeWidgetItem();
        child->setText(0, job->description());
        child->setIcon(0, QIcon::fromTheme(job->statusIcon()));
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
        currentJobItem()->setIcon(0, QIcon::fromTheme(job->statusIcon()));

    setCurrentJobItem(nullptr);

    const int current = dialogWidget().progressTotal().value();
    dialogWidget().progressTotal().setValue(current + 1);

    setParentTitle(op->description());
    updateReport(true);
}

void ApplyProgressDialog::onOpFinished(int num, Operation* op)
{
    if (currentOpItem()) {
        currentOpItem()->setText(0, opDesc(num, *op));
        currentOpItem()->setIcon(0, QIcon::fromTheme(op->statusIcon()));
    }

    setCurrentOpItem(nullptr);

    setStatus(op->description());

    dialogWidget().progressSub().setValue(op->totalProgress());
    updateReport(true);
}

void ApplyProgressDialog::setParentTitle(const QString& s)
{
    const int percent = dialogWidget().progressTotal().value() * 100 / dialogWidget().progressTotal().maximum();
    mainWindow(this)->setWindowTitle(QString::number(percent) + QStringLiteral("% - ") + s + QStringLiteral(" - ") + savedParentTitle());
}

void ApplyProgressDialog::setStatus(const QString& s)
{
    setWindowTitle(s);
    dialogWidget().status().setText(s);

    setParentTitle(s);
}

QString ApplyProgressDialog::opDesc(int num, const Operation& op) const
{
    return xi18nc("@info:progress", "[%1/%2] - %3: %4", num, operationRunner().numOperations(), op.statusText(), op.description());
}

void ApplyProgressDialog::addTaskOutput(int num, const Operation& op)
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setIcon(0, QIcon::fromTheme(op.statusIcon()));
    item->setText(0, opDesc(num, op));
    item->setText(1, QTime(0, 0).toString(timeFormat()));

    QFont f;
    f.setWeight(QFont::Bold);
    item->setFont(0, f);
    item->setFont(1, f);

    item->setData(0, Qt::UserRole, reinterpret_cast<qulonglong>(&op));
    dialogWidget().treeTasks().addTopLevelItem(item);
    dialogWidget().treeTasks().scrollToBottom();
    setCurrentOpItem(item);
}

void ApplyProgressDialog::onSecondElapsed()
{
    if (currentJobItem()) {
        QTime t = QTime::fromString(currentJobItem()->text(1), timeFormat()).addSecs(1);
        currentJobItem()->setText(1, t.toString(timeFormat()));
    }

    if (currentOpItem()) {
        QTime t = QTime::fromString(currentOpItem()->text(1), timeFormat()).addSecs(1);
        currentOpItem()->setText(1, t.toString(timeFormat()));;
    }

    const QTime outputTime = QTime(0, 0).addMSecs(time().elapsed());
    dialogWidget().totalTime().setText(xi18nc("@info:progress", "Total Time: %1", outputTime.toString(timeFormat())));
}

void ApplyProgressDialog::keyPressEvent(QKeyEvent* e)
{
    e->accept();

    switch (e->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (okButton->isEnabled())
            onOkButton();
        break;

    case Qt::Key_Escape:
        cancelButton->isEnabled() ? onCancelButton() : onOkButton();
        break;

    default:
        break;
    }
}

void ApplyProgressDialog::saveReport()
{
    const QUrl url = QFileDialog::getSaveFileUrl();

    if (url.isEmpty())
        return;

    QTemporaryFile tempFile;

    if (tempFile.open()) {
        QTextStream s(&tempFile);

        HtmlReport html;

        s << html.header()
          << report().toHtml()
          << html.footer();

        tempFile.close();

        KIO::CopyJob* job = KIO::move(QUrl::fromLocalFile(tempFile.fileName()), url, KIO::HideProgressInfo);
        job->exec();
        if (job->error())
            job->uiDelegate()->showErrorMessage();
    } else
        KMessageBox::error(this, xi18nc("@info", "Could not create temporary file when trying to save to <filename>%1</filename>.", url.fileName()), xi18nc("@title:window", "Could Not Save Report."));
}

void ApplyProgressDialog::browserReport()
{
    QTemporaryFile file;

    file.setFileTemplate(QStringLiteral("/tmp/") + QCoreApplication::applicationName() + QStringLiteral("-XXXXXX.html"));
    file.setAutoRemove(false);

    if (file.open()) {
        QTextStream s(&file);

        HtmlReport html;

        s << html.header()
          << report().toHtml()
          << html.footer();

        file.setPermissions(QFile::ReadOwner | QFile::WriteOwner);

        auto *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(file.fileName()), QStringLiteral("text/html"), this);
        job->start();
    } else
        KMessageBox::error(this, xi18nc("@info", "Could not create temporary file <filename>%1</filename> for writing.", file.fileName()), i18nc("@title:window", "Could Not Launch Browser."));
}
