/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/smartdialog.h"
#include "gui/smartdialogwidget.h"

#include <core/device.h>
#include <core/smartstatus.h>
#include <core/smartattribute.h>

#include <util/helpers.h>
#include <util/htmlreport.h>

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontDatabase>
#include <QPushButton>
#include <QStyle>
#include <QTemporaryFile>
#include <QTextDocumentFragment>
#include <QTextStream>
#include <QTreeWidgetItem>
#include <QtGlobal>

#include <KConfigGroup>
#include <KFormat>
#include <KLocalizedString>
#include <KIO/CopyJob>
#include <KJobUiDelegate>
#include <KMessageBox>
#include <KSharedConfig>

#include <sys/utsname.h>
#include <unistd.h>

#include <config.h>

static const QList<SmartNvmeHealth::Field> nvmeHealthFields = {
    SmartNvmeHealth::Field::CriticalWarning,
    SmartNvmeHealth::Field::AvailableSpare,
    SmartNvmeHealth::Field::AvailableSpareThreshold,
    SmartNvmeHealth::Field::PercentageUsed,
    SmartNvmeHealth::Field::DataUnitsRead,
    SmartNvmeHealth::Field::DataUnitsWritten,
    SmartNvmeHealth::Field::HostReadCommands,
    SmartNvmeHealth::Field::HostWriteCommands,
    SmartNvmeHealth::Field::ControllerBusyTime,
    SmartNvmeHealth::Field::UnsafeShutdowns,
    SmartNvmeHealth::Field::MediaErrors,
    SmartNvmeHealth::Field::ErrorLogEntries,
};

static QString unknownValue()
{
    return xi18nc("@label SMART value not reported by the device", "unknown");
}

static QString temperature(const SmartDetails& d)
{
    return d.temp() ? SmartStatus::tempToString(*d.temp()) : unknownValue();
}

static QString powerCycles(const SmartDetails& d)
{
    return d.powerCycles() ? QLocale().toString(*d.powerCycles()) : unknownValue();
}

/** Creates a new SmartDialog
    @param parent pointer to the parent widget
    @param d the Device
*/
SmartDialog::SmartDialog(QWidget* parent, Device& d) :
    QDialog(parent),
    m_Device(d),
    m_SmartDetails(d.deviceNode()),
    m_DialogWidget(new SmartDialogWidget(this))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&dialogWidget());
    setWindowTitle(xi18nc("@title:window", "SMART Properties: <filename>%1</filename>", device().deviceNode()));

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Close);
    buttonBox->button(QDialogButtonBox::Save)->setText(xi18nc("@action:button", "Save SMART Report"));
    buttonBox->button(QDialogButtonBox::Save)->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    mainLayout->addWidget(buttonBox);

    setupDialog();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("smartDialog"));
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a SmartDialog */
SmartDialog::~SmartDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("smartDialog"));
    kcg.writeEntry("Geometry", saveGeometry());
}

void SmartDialog::setupDialog()
{
    if (device().smartStatus().isValid()) {
        if (device().smartStatus().status()) {
            dialogWidget().statusText().setText(xi18nc("@label SMART disk status", "good"));
            dialogWidget().statusIcon().setPixmap(QIcon::fromTheme(QStringLiteral("dialog-ok"))
                    .pixmap(QApplication::style()->pixelMetric(QStyle::PixelMetric::PM_SmallIconSize)));
        } else {
            dialogWidget().statusText().setText(xi18nc("@label SMART disk status", "BAD"));
            dialogWidget().statusIcon().setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning"))
                    .pixmap(QApplication::style()->pixelMetric(QStyle::PixelMetric::PM_SmallIconSize)));
        }

        dialogWidget().modelName().setText(device().smartStatus().modelName());
        dialogWidget().firmware().setText(device().smartStatus().firmware());
        dialogWidget().serialNumber().setText(device().smartStatus().serial());

        dialogWidget().temperature().setText(temperature(m_SmartDetails));
        const QString badSectors = device().smartStatus().badSectors() > 0
                                   ? QLocale().toString(device().smartStatus().badSectors())
                                   : xi18nc("@label SMART number of bad sectors", "none");
        dialogWidget().badSectors().setText(badSectors);
        dialogWidget().badSectors().setVisible(!m_SmartDetails.isNvme());
        dialogWidget().badSectorsLabel().setVisible(!m_SmartDetails.isNvme());
        dialogWidget().poweredOn().setText(m_SmartDetails.poweredOn()
                                           ? KFormat().formatDuration(*m_SmartDetails.poweredOn())
                                           : unknownValue());
        dialogWidget().powerCycles().setText(powerCycles(m_SmartDetails));
        dialogWidget().overallAssessment().setText(SmartStatus::overallAssessmentToString(device().smartStatus().overall()));
        dialogWidget().selfTests().setText(SmartStatus::selfTestStatusToString(device().smartStatus().selfTestStatus()));

        dialogWidget().treeSmartAttributes().clear();

        const QFont f = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
        const QString size = f.pixelSize() != -1 ? QStringLiteral("%1px").arg(f.pixelSize()) : QStringLiteral("%1pt").arg(f.pointSize());

        const QString st = QStringLiteral("<span style=\"font-family:%1;font-size:%2;\">").arg(f.family()).arg(size);

        for (const auto &a : device().smartStatus().attributes()) {
            QTreeWidgetItem* item = new QTreeWidgetItem(
                QStringList()
                << QLocale().toString(a.id())
                << QStringLiteral("<b>%1</b><br/>%2").arg(a.name()).arg(st + a.desc() + QStringLiteral("</span>"))
                << (a.failureType() == SmartAttribute::FailureType::PreFailure ? xi18nc("@item:intable", "Pre-Failure") : xi18nc("@item:intable", "Old-Age"))
                << (a.updateType() == SmartAttribute::UpdateType::Online ? xi18nc("@item:intable", "Online") : xi18nc("@item:intable", "Offline"))
                << QLocale().toString(a.worst())
                << QLocale().toString(a.current())
                << QLocale().toString(a.threshold())
                << a.raw()
                << a.assessmentToString()
                << a.value()
            );
            item->setSizeHint(0, QSize(0, 64));
            item->setToolTip(1, QTextDocumentFragment::fromHtml(a.desc()).toPlainText());
            dialogWidget().treeSmartAttributes().addTopLevelItem(item);
        }

        const bool nvme = m_SmartDetails.isNvme();
        dialogWidget().treeSmartAttributes().setVisible(!nvme);
        dialogWidget().treeNvmeHealth().setVisible(nvme);
        dialogWidget().treeNvmeHealth().clear();

        if (nvme) {
            const SmartNvmeHealth& health = m_SmartDetails.nvmeHealth();
            for (const auto field : nvmeHealthFields) {
                if (!health.contains(field))
                    continue;
                dialogWidget().treeNvmeHealth().addTopLevelItem(new QTreeWidgetItem(
                    QStringList() << SmartNvmeHealth::fieldName(field) << health.valueToString(field)));
            }
            dialogWidget().treeNvmeHealth().resizeColumnToContents(0);
        }
    } else {
        dialogWidget().statusText().setText(xi18nc("@label", "(unknown)"));
        dialogWidget().treeNvmeHealth().setVisible(false);
    }

    setMinimumSize(dialogWidget().size());
    resize(dialogWidget().size());
}

void SmartDialog::setupConnections()
{
    connect(buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, this, &SmartDialog::saveSmartReport);
    connect(buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked, this, &SmartDialog::close);
}

QString SmartDialog::toHtml() const
{
    QString rval;
    QTextStream s(&rval);

    if (device().smartStatus().status())
        s << HtmlReport::tableLine(i18n("SMART status:"), xi18nc("@label SMART disk status", "good"));
    else
        s << HtmlReport::tableLine(i18n("SMART status:"), xi18nc("@label SMART disk status", "BAD"));

    const QString badSectors = device().smartStatus().badSectors() > 0
                               ? QLocale().toString(device().smartStatus().badSectors())
                               : i18nc("@label SMART number of bad sectors", "none");

    s << HtmlReport::tableLine(i18n("Model:"), device().smartStatus().modelName())
      << HtmlReport::tableLine(i18n("Serial number:"), device().smartStatus().serial())
      << HtmlReport::tableLine(i18n("Firmware revision:"), device().smartStatus().firmware())
      << HtmlReport::tableLine(i18n("Temperature:"), temperature(m_SmartDetails));

    if (!m_SmartDetails.isNvme())
        s << HtmlReport::tableLine(i18n("Bad sectors:"), badSectors);

    s << HtmlReport::tableLine(i18n("Powered on for:"), m_SmartDetails.poweredOn()
                                                        ? KFormat().formatSpelloutDuration(*m_SmartDetails.poweredOn())
                                                        : unknownValue())
      << HtmlReport::tableLine(i18n("Power cycles:"), powerCycles(m_SmartDetails))
      << HtmlReport::tableLine(i18n("Self tests:"), SmartStatus::selfTestStatusToString(device().smartStatus().selfTestStatus()))
      << HtmlReport::tableLine(i18n("Overall assessment:"), SmartStatus::overallAssessmentToString(device().smartStatus().overall()));

    if (m_SmartDetails.isNvme()) {
        const SmartNvmeHealth& health = m_SmartDetails.nvmeHealth();
        for (const auto field : nvmeHealthFields) {
            if (health.contains(field))
                s << HtmlReport::tableLine(SmartNvmeHealth::fieldName(field) + QLatin1Char(':'), health.valueToString(field));
        }
    }

    s << "</table><br/>";

    if (device().smartStatus().isValid() && !m_SmartDetails.isNvme()) {

        const QFont f = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
        const QString size = f.pixelSize() != -1 ? QStringLiteral("%1px").arg(f.pixelSize()) : QStringLiteral("%1pt").arg(f.pointSize());

        const QString st = QStringLiteral("<span style=\"font-family:%1;font-size:%2;\">").arg(f.family()).arg(size);

        s << "<table>\n";

        for (const auto &a : device().smartStatus().attributes()) {
            s << "<tr>\n";

            s << "<td>" << QLocale().toString(a.id()) << "</td>\n"
              << "<td>" << QStringLiteral("<b>%1</b><br/>%2").arg(a.name()).arg(st + a.desc() + QStringLiteral("</span>")) << "</td>\n"
              << "<td>" << (a.failureType() == SmartAttribute::FailureType::PreFailure ? xi18nc("@item:intable", "Pre-Failure") : xi18nc("@item:intable", "Old-Age")) << "</td>\n"
              << "<td>" << (a.updateType() == SmartAttribute::UpdateType::Online ? xi18nc("@item:intable", "Online") : xi18nc("@item:intable", "Offline")) << "</td>\n"
              << "<td>" << QLocale().toString(a.worst()) << "</td>\n"
              << "<td>" << QLocale().toString(a.current()) << "</td>\n"
              << "<td>" << QLocale().toString(a.threshold()) << "</td>\n"
              << "<td>" << a.raw() << "</td>\n"
              << "<td>" << a.assessmentToString() << "</td>\n"
              << "<td>" << a.value() << "</td>\n";

            s << "</tr>\n";
        }

        s << "</table>\n";
    } else if (!device().smartStatus().isValid())
        s << "(unknown)";

    s.flush();

    return rval;
}

void SmartDialog::saveSmartReport()
{
    const QUrl url = QFileDialog::getSaveFileUrl(this, QString(), QUrl(), tr("HTML-Files (*.htm *.html);;All Files (*)"));

    if (url.isEmpty())
        return;

    QTemporaryFile tempFile;

    if (tempFile.open()) {
        QTextStream s(&tempFile);

        HtmlReport html;

        s << html.header()
          << toHtml()
          << html.footer();

        tempFile.close();

        KIO::CopyJob* job = KIO::move(QUrl::fromLocalFile(tempFile.fileName()), url, KIO::HideProgressInfo);
        job->exec();
        if (job->error())
            job->uiDelegate()->showErrorMessage();
    } else
        KMessageBox::error(this, xi18nc("@info", "Could not create temporary file when trying to save to <filename>%1</filename>.", url.fileName()), xi18nc("@title:window", "Could Not Save SMART Report."));

}
