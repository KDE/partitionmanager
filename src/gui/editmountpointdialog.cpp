/***************************************************************************
 *   Copyright (C) 2009 by Volker Lanz <vl@fidra.de>                       *
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

#include "gui/editmountpointdialog.h"
#include "gui/editmountpointdialogwidget.h"

#include "core/partition.h"

#include <KMessageBox>
#include <KDebug>

EditMountPointDialog::EditMountPointDialog(QWidget* parent, Partition& p) :
	KDialog(parent),
	m_Partition(p),
	m_DialogWidget(new EditMountPointDialogWidget(this, partition()))
{
	setMainWidget(&widget());
}

void EditMountPointDialog::accept()
{
	if (widget().editPath().text().isEmpty())
	{
		KMessageBox::sorry(this, i18n("The path can not be empty\n\nPlease fill in a valid path or \"none\" if the file system does not require a path to be set (e.g. swap)."), i18n("Error"));
		widget().editPath().setFocus(Qt::OtherFocusReason);
		return;
	}

#if 0
	partition().setName(widget().editName().text());
	partition().setPath(widget().editPath().text());
	partition().setType(widget().type());
	partition().setOptions(widget().options());
	partition().setDumpFreq(widget().spinDumpFreq().value());
	partition().setPassNumber(widget().spinPassNumber().value());
#endif

	KDialog::accept();
}
