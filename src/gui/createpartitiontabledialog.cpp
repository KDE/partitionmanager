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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "gui/createpartitiontabledialog.h"
#include "gui/createpartitiontablewidget.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <config.h>

CreatePartitionTableDialog::CreatePartitionTableDialog(QWidget* parent, const Device& d) :
	KDialog(parent),
	m_DialogWidget(new CreatePartitionTableWidget(this)),
	m_Device(d)
{
	setMainWidget(&widget());
	setCaption(i18nc("@title:window", "Create a New Partition Table on <filename>%1</filename>", device().deviceNode()));
	setButtonText(KDialog::Ok, i18nc("@action:button", "&Create New Partition Table"));

	connect(&widget().radioMSDOS(), SIGNAL(toggled(bool)), SLOT(onMSDOSToggled(bool)));
}

PartitionTable::TableType CreatePartitionTableDialog::type() const
{
	if (widget().radioGPT().isChecked())
		return PartitionTable::gpt;

	if (widget().radioMSDOS().isChecked() && Config::useCylinderAlignment() == true)
		return PartitionTable::msdos;

	return PartitionTable::msdos_sectorbased;
}

void CreatePartitionTableDialog::onMSDOSToggled(bool on)
{
	if (on && device().totalSectors() > 0xffffffff)
	{
		if (KMessageBox::warningContinueCancel(this,
				i18nc("@info",
					"<para>Do you really want to create an MS-Dos partition table on <filename>%1</filename>?</para>"
					"<para>This device has more than 2^32 sectors. That is the most the MS-Dos partition table type supports, so you will not be able to use the whole device.</para>", device().deviceNode()),
				i18nc("@title:window", "Really Create MS-Dos Partition Table Type?"),
				KGuiItem(i18nc("@action:button", "Create MS-Dos Type"), "arrow-right"),
				KStandardGuiItem::cancel(), "reallyCreateMSDOSOnLargeDevice") == KMessageBox::Cancel)
		{
			widget().radioGPT().setChecked(true);
		}
	}
}

