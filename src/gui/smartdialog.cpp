/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#include "gui/smartdialog.h"
#include "gui/smartdialogwidget.h"

#include "core/device.h"
#include "core/smartstatus.h"
#include "core/smartattribute.h"

#include "util/helpers.h"
#include "util/htmlreport.h"

#include <kdebug.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <kio/jobuidelegate.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kicon.h>
#include <kglobalsettings.h>
#include <kglobal.h>

#include <QTreeWidgetItem>
#include <QTextStream>
#include <QTextDocument>
#include <qglobal.h>

#include <sys/utsname.h>
#include <unistd.h>

#include <config.h>

/** Creates a new SmartDialog
	@param parent pointer to the parent widget
	@param d the Device
*/
SmartDialog::SmartDialog(QWidget* parent, Device& d) :
	KDialog(parent),
	m_Device(d),
	m_DialogWidget(new SmartDialogWidget(this))
{
	setMainWidget(&dialogWidget());
	setCaption(i18nc("@title:window", "SMART Properties: <filename>%1</filename>", device().deviceNode()));
	setButtons(Close|User1);
	setButtonText(User1, i18nc("@action:button", "Save SMART Report"));
	button(User1)->setIcon(KIcon("document-save"));

	setupDialog();
	setupConnections();

	restoreDialogSize(KConfigGroup(KGlobal::config(), "smartDialog"));
}

/** Destroys a SmartDialog */
SmartDialog::~SmartDialog()
{
	KConfigGroup kcg(KGlobal::config(), "smartDialog");
	saveDialogSize(kcg);
}

void SmartDialog::setupDialog()
{
	if (device().smartStatus().isValid())
	{
		if (device().smartStatus().status())
		{
			dialogWidget().statusText().setText(i18nc("@label SMART disk status", "good"));
			dialogWidget().statusIcon().setVisible(false);
		}
		else
		{
			dialogWidget().statusText().setText(i18nc("@label SMART disk status", "BAD"));
			dialogWidget().statusIcon().setPixmap(SmallIcon("dialog-warning"));
		}

		dialogWidget().modelName().setText(device().smartStatus().modelName());
		dialogWidget().firmware().setText(device().smartStatus().firmware());
		dialogWidget().serialNumber().setText(device().smartStatus().serial());

		dialogWidget().temperature().setText(SmartStatus::tempToString(device().smartStatus().temp()));
		const QString badSectors = device().smartStatus().badSectors() > 0
				? KGlobal::locale()->formatNumber(device().smartStatus().badSectors(), 0)
				: i18nc("@label SMART number of bad sectors", "none");
		dialogWidget().badSectors().setText(badSectors);
		dialogWidget().poweredOn().setText(KGlobal::locale()->formatDuration(device().smartStatus().poweredOn()));
		dialogWidget().powerCycles().setText(KGlobal::locale()->formatNumber(device().smartStatus().powerCycles(), 0));
		dialogWidget().overallAssessment().setText(SmartStatus::overallAssessmentToString(device().smartStatus().overall()));
		dialogWidget().selfTests().setText(SmartStatus::selfTestStatusToString(device().smartStatus().selfTestStatus()));

		dialogWidget().treeSmartAttributes().clear();

		const QFont f = KGlobalSettings::smallestReadableFont();
		const QString size = f.pixelSize() != -1 ? QString("%1px").arg(f.pixelSize()) : QString("%1pt").arg(f.pointSize());

		const QString st = QString("<span style=\"font-family:%1;font-size:%2;\">").arg(f.family()).arg(size);

		foreach (const SmartAttribute& a, device().smartStatus().attributes())
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(
				QStringList()
					<< KGlobal::locale()->formatNumber(a.id(), 0)
					<< QString("<b>%1</b><br/>%2").arg(a.name()).arg(st + a.desc() + "</span>")
					<< (a.failureType() == SmartAttribute::PreFailure ? i18nc("@item:intable", "Pre-Failure") : i18nc("@item:intable", "Old-Age"))
					<< (a.updateType() == SmartAttribute::Online ? i18nc("@item:intable", "Online") : i18nc("@item:intable", "Offline"))
					<< KGlobal::locale()->formatNumber(a.worst(), 0)
					<< KGlobal::locale()->formatNumber(a.current(), 0)
					<< KGlobal::locale()->formatNumber(a.threshold(), 0)
					<< a.raw()
					<< a.assessmentToString()
					<< a.value()
			);
			item->setSizeHint(0, QSize(0, 64));
			dialogWidget().treeSmartAttributes().addTopLevelItem(item);
		}
	}
	else
		dialogWidget().statusText().setText(i18nc("@label", "(unknown)"));

	setMinimumSize(dialogWidget().size());
	resize(dialogWidget().size());
}

void SmartDialog::setupConnections()
{
	connect(this, SIGNAL(user1Clicked()), SLOT(saveSmartReport()));
}

QString SmartDialog::toHtml() const
{
	QString rval;
	QTextStream s(&rval);

	if (device().smartStatus().status())
		s << HtmlReport::tableLine(i18n("SMART status:"), i18nc("@label SMART disk status", "good"));
	else
		s << HtmlReport::tableLine(i18n("SMART status:"), i18nc("@label SMART disk status", "BAD"));

	const QString badSectors = device().smartStatus().badSectors() > 0
			? KGlobal::locale()->formatNumber(device().smartStatus().badSectors(), 0)
			: i18nc("@label SMART number of bad sectors", "none");

	s << HtmlReport::tableLine(i18n("Model:"), device().smartStatus().modelName())
		<< HtmlReport::tableLine(i18n("Serial number:"), device().smartStatus().serial())
		<< HtmlReport::tableLine(i18n("Firmware revision:"), device().smartStatus().firmware())
		<< HtmlReport::tableLine(i18n("Temperature:"), SmartStatus::tempToString(device().smartStatus().temp()))
		<< HtmlReport::tableLine(i18n("Bad sectors:"), badSectors)
		<< HtmlReport::tableLine(i18n("Powered on for:"), KGlobal::locale()->formatDuration(device().smartStatus().poweredOn()))
		<< HtmlReport::tableLine(i18n("Power cycles:"), KGlobal::locale()->formatNumber(device().smartStatus().powerCycles(), 0))
		<< HtmlReport::tableLine(i18n("Self tests:"), SmartStatus::selfTestStatusToString(device().smartStatus().selfTestStatus()))
		<< HtmlReport::tableLine(i18n("Overall assessment:"), SmartStatus::overallAssessmentToString(device().smartStatus().overall()));

	s << "</table><br/>";

	if (device().smartStatus().isValid())
	{

		const QFont f = KGlobalSettings::smallestReadableFont();
		const QString size = f.pixelSize() != -1 ? QString("%1px").arg(f.pixelSize()) : QString("%1pt").arg(f.pointSize());

		const QString st = QString("<span style=\"font-family:%1;font-size:%2;\">").arg(f.family()).arg(size);

		s << "<table>\n";

		foreach (const SmartAttribute& a, device().smartStatus().attributes())
		{
			s << "<tr>\n";

			s << "<td>" << KGlobal::locale()->formatNumber(a.id(), 0) << "</td>\n"
				<< "<td>" << QString("<b>%1</b><br/>%2").arg(a.name()).arg(st + a.desc() + "</span>") << "</td>\n"
				<< "<td>" << (a.failureType() == SmartAttribute::PreFailure ? i18nc("@item:intable", "Pre-Failure") : i18nc("@item:intable", "Old-Age")) << "</td>\n"
				<< "<td>" << (a.updateType() == SmartAttribute::Online ? i18nc("@item:intable", "Online") : i18nc("@item:intable", "Offline")) << "</td>\n"
				<< "<td>" << KGlobal::locale()->formatNumber(a.worst(), 0) << "</td>\n"
				<< "<td>" << KGlobal::locale()->formatNumber(a.current(), 0) << "</td>\n"
				<< "<td>" << KGlobal::locale()->formatNumber(a.threshold(), 0) << "</td>\n"
				<< "<td>" << a.raw() << "</td>\n"
				<< "<td>" << a.assessmentToString() << "</td>\n"
				<< "<td>" << a.value() << "</td>\n";

			s << "</tr>\n";
		}

		s << "</table>\n";
	}
	else
		s << "(unknown)";

	s.flush();

	return rval;
}

void SmartDialog::saveSmartReport()
{
	const KUrl url = KFileDialog::getSaveUrl(KUrl("kfiledialog://saveSMARTReport"));

	if (url.isEmpty())
		return;

	KTemporaryFile tempFile;

	if (tempFile.open())
	{
		QTextStream s(&tempFile);

		HtmlReport html;

		s << html.header()
			<< toHtml()
			<< html.footer();

		tempFile.close();

		KIO::CopyJob* job = KIO::move(tempFile.fileName(), url, KIO::HideProgressInfo);
		if (!KIO::NetAccess::synchronousRun(job, NULL))
			job->ui()->showErrorMessage();
	}
	else
		KMessageBox::sorry(this, i18nc("@info", "Could not create temporary file when trying to save to <filename>%1</filename>.", url.fileName()), i18nc("@title:window", "Could Not Save SMART Report."));

}
