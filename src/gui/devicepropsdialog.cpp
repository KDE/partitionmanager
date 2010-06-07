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

#include "gui/devicepropsdialog.h"
#include "gui/devicepropswidget.h"

#include "gui/smartdialog.h"

#include "core/device.h"
#include "core/partitiontable.h"
#include "core/smartstatus.h"

#include "util/capacity.h"
#include "util/helpers.h"

#include <kdebug.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include <QTreeWidgetItem>
#include <QPointer>

/** Creates a new DevicePropsDialog
	@param parent pointer to the parent widget
	@param d the Device to show properties for
*/
DevicePropsDialog::DevicePropsDialog(QWidget* parent, Device& d) :
	KDialog(parent),
	m_Device(d),
	m_DialogWidget(new DevicePropsWidget(this))
{
	setMainWidget(&dialogWidget());
	setCaption(i18nc("@title:window", "Device Properties: <filename>%1</filename>", device().deviceNode()));

	setupDialog();
	setupConnections();

	restoreDialogSize(KConfigGroup(KGlobal::config(), "devicePropsDialog"));
}

/** Destroys a DevicePropsDialog */
DevicePropsDialog::~DevicePropsDialog()
{
	KConfigGroup kcg(KGlobal::config(), "devicePropsDialog");
	saveDialogSize(kcg);
}

void DevicePropsDialog::setupDialog()
{
	setDefaultButton(KDialog::Cancel);
	enableButtonOk(false);
	button(KDialog::Cancel)->setFocus();

	QString type = "---";
	QString maxPrimaries = "---";

	if (device().partitionTable() != NULL)
	{
		type = (device().partitionTable()->isReadOnly())
			? i18nc("@label device", "%1 (read only)", device().partitionTable()->typeName())
			: device().partitionTable()->typeName();
		maxPrimaries = QString("%1/%2").arg(device().partitionTable()->numPrimaries()).arg(device().partitionTable()->maxPrimaries());

		dialogWidget().partTableWidget().setReadOnly(true);
		dialogWidget().partTableWidget().setPartitionTable(device().partitionTable());

		if (device().partitionTable()->type() == PartitionTable::msdos)
			dialogWidget().radioCylinderBased().setChecked(true);
		else if (device().partitionTable()->type() == PartitionTable::msdos_sectorbased)
			dialogWidget().radioSectorBased().setChecked(true);
		else
			dialogWidget().hideTypeRadioButtons();
	}
	else
	{
		dialogWidget().partTableWidget().setVisible(false);
		dialogWidget().hideTypeRadioButtons();
	}

	dialogWidget().capacity().setText(Capacity(device()).toString(Capacity::AppendUnit | Capacity::AppendBytes));

	const QString cyls = KGlobal::locale()->formatNumber(device().cylinders(), 0);
	const QString heads = QString::number(device().heads());
	const QString sectors = KGlobal::locale()->formatNumber(device().sectorsPerTrack(), 0);
	dialogWidget().chs().setText(QString("%1/%2/%3").arg(cyls).arg(heads).arg(sectors));

	dialogWidget().cylinderSize().setText(i18ncp("@label", "1 Sector", "%1 Sectors", device().cylinderSize()));
	dialogWidget().primariesMax().setText(maxPrimaries);
	dialogWidget().logicalSectorSize().setText(Capacity(device().logicalSectorSize()).toString(Capacity::Byte, Capacity::AppendUnit));
	dialogWidget().physicalSectorSize().setText(Capacity(device().physicalSectorSize()).toString(Capacity::Byte, Capacity::AppendUnit));
	dialogWidget().totalSectors().setText(KGlobal::locale()->formatNumber(device().totalSectors(), 0));
	dialogWidget().type().setText(type);

	if (device().smartStatus().isValid())
	{
		if (device().smartStatus().status())
		{
			dialogWidget().smartStatusText().setText(i18nc("@label SMART disk status", "good"));
			dialogWidget().smartStatusIcon().setPixmap(SmallIcon("dialog-ok"));
		}
		else
		{
			dialogWidget().smartStatusText().setText(i18nc("@label SMART disk status", "BAD"));
			dialogWidget().smartStatusIcon().setPixmap(SmallIcon("dialog-warning"));
		}
	}
	else
	{
		dialogWidget().smartStatusText().setText(i18nc("@label", "(unknown)"));
		dialogWidget().smartStatusIcon().setVisible(false);
		dialogWidget().buttonSmartMore().setVisible(false);
	}

	setMinimumSize(dialogWidget().size());
	resize(dialogWidget().size());
}

void DevicePropsDialog::setupConnections()
{
	connect(&dialogWidget().radioSectorBased(), SIGNAL(toggled(bool)), SLOT(setDirty(bool)));
	connect(&dialogWidget().radioCylinderBased(), SIGNAL(toggled(bool)), SLOT(setDirty(bool)));
	connect(&dialogWidget().buttonSmartMore(), SIGNAL(clicked(bool)), SLOT(onButtonSmartMore(bool)));
}

void DevicePropsDialog::setDirty(bool)
{
	setDefaultButton(KDialog::Ok);
	enableButtonOk(true);
}

bool DevicePropsDialog::cylinderBasedAlignment() const
{
	return dialogWidget().radioCylinderBased().isChecked();
}

bool DevicePropsDialog::sectorBasedAlignment() const
{
	return dialogWidget().radioSectorBased().isChecked();
}

void DevicePropsDialog::onButtonSmartMore(bool)
{
	QPointer<SmartDialog> dlg = new SmartDialog(this, device());
	dlg->exec();
	delete dlg;
}
