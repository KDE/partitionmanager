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

#include <config.h>

CreatePartitionTableDialog::CreatePartitionTableDialog(QWidget* parent, const Device& d) :
	KDialog(parent),
	m_DialogWidget(new CreatePartitionTableWidget(this)),
	m_Device(d)
{
	setMainWidget(&widget());
	setCaption(i18nc("@title:window", "Create a New Partition Table on <filename>%1</filename>", device().deviceNode()));
	setButtonText(KDialog::Ok, i18nc("@action:button", "&Create New Partition Table"));
}

PartitionTable::TableType CreatePartitionTableDialog::type() const
{
	if (widget().radioGPT().isChecked())
		return PartitionTable::gpt;

	if (widget().radioMSDOS().isChecked() && Config::useCylinderAlignment() == true)
		return PartitionTable::msdos;

	return PartitionTable::msdos_sectorbased;
}
