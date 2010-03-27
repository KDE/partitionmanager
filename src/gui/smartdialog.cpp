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

#include <kdebug.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include <QTreeWidgetItem>

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
	setButtons(Close);

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
					<< QString("<b>%1</b><br>%2").arg(a.name()).arg(st + a.desc() + "</style>")
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
}
