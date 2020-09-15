/*************************************************************************
 *  Copyright (C) 2008, 2012 by Volker Lanz <vl@fidra.de>                *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#ifndef APPLYPROGRESSDIALOG_H
#define APPLYPROGRESSDIALOG_H

#include <QDialog>
#include <QString>
#include <QElapsedTimer>
#include <QTimer>

class OperationRunner;
class Operation;
class Job;
class ApplyProgressDialogWidget;
class ApplyProgressDetailsWidget;
class Report;

class QDialogButtonBox;
class QTreeWidgetItem;
class QCloseEvent;
class QKeyEvent;

/** Show progress.

    The progress dialog telling the user about the progress of the Operations that are being applied.

    @author Volker Lanz <vl@fidra.de>
*/
class ApplyProgressDialog : public QDialog
{
    Q_DISABLE_COPY(ApplyProgressDialog)

public:
    ApplyProgressDialog(QWidget* parent, OperationRunner& orunner);
    ~ApplyProgressDialog();

public:
    void show();

    Report& report() {
        Q_ASSERT(m_Report);    /**< @return the Report object for this dialog */
        return *m_Report;
    }
    const Report& report() const {
        Q_ASSERT(m_Report);    /**< @return the Report object for this dialog */
        return *m_Report;
    }

protected:
    void onAllOpsFinished();
    void onAllOpsCancelled();
    void onAllOpsError();
    void onCancelButton();
    void onDetailsButton();
    void onOkButton();
    void onOpStarted(int num, Operation* op);
    void onOpFinished(int num, Operation* op);
    void onSecondElapsed();
    void saveReport();
    void browserReport();
    void updateReportUnforced();
    void updateReport(bool force = false);

    void onJobStarted(Job* job, Operation* op);
    void onJobFinished(Job* job, Operation* op);
    void toggleDetails();

    void closeEvent(QCloseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;

    void setupConnections();

    const OperationRunner& operationRunner() const {
        return m_OperationRunner;
    }

    ApplyProgressDialogWidget
    & dialogWidget() {
        Q_ASSERT(m_ProgressDialogWidget);
        return *m_ProgressDialogWidget;
    }
    const ApplyProgressDialogWidget
    & dialogWidget() const {
        Q_ASSERT(m_ProgressDialogWidget);
        return *m_ProgressDialogWidget;
    }

    ApplyProgressDetailsWidget
    & detailsWidget() {
        Q_ASSERT(m_ProgressDetailsWidget);
        return *m_ProgressDetailsWidget;
    }
    const ApplyProgressDetailsWidget
    & detailsWidget() const {
        Q_ASSERT(m_ProgressDetailsWidget);
        return *m_ProgressDetailsWidget;
    }

    void setStatus(const QString& s);

    void setParentTitle(const QString& s);

    void addTaskOutput(int num, const Operation& op);

    QString opDesc(int num, const Operation& op) const;

    void resetReport();

    void allOpsDone(const QString& msg);

    QElapsedTimer& time() {
        return m_ElapsedTimer;
    }

    QTimer& timer() {
        return m_Timer;
    }
    const QTimer& timer() const {
        return m_Timer;
    }

    const QString& savedParentTitle() const {
        return m_SavedParentTitle;
    }

    void setCurrentOpItem(QTreeWidgetItem* item) {
        m_CurrentOpItem = item;
    }
    QTreeWidgetItem* currentOpItem() {
        return m_CurrentOpItem;
    }

    void setCurrentJobItem(QTreeWidgetItem* item) {
        m_CurrentJobItem = item;
    }
    QTreeWidgetItem* currentJobItem() {
        return m_CurrentJobItem;
    }

    int lastReportUpdate() const {
        return m_LastReportUpdate;
    }
    void setLastReportUpdate(int t) {
        m_LastReportUpdate = t;
    }

    static const QString& timeFormat() {
        return m_TimeFormat;
    }

private:
    ApplyProgressDialogWidget* m_ProgressDialogWidget;
    ApplyProgressDetailsWidget* m_ProgressDetailsWidget;
    const OperationRunner& m_OperationRunner;
    Report* m_Report;
    QString m_SavedParentTitle;
    QTimer m_Timer;
    QElapsedTimer m_ElapsedTimer;
    QTreeWidgetItem* m_CurrentOpItem;
    QTreeWidgetItem* m_CurrentJobItem;
    int m_LastReportUpdate;

    QDialogButtonBox* dialogButtonBox;
    QPushButton* okButton;
    QPushButton* cancelButton;
    QPushButton* detailsButton;

    static const QString m_TimeFormat;
};

#endif
