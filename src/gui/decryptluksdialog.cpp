/***************************************************************************
 *   Copyright (C) 2013 by Andrius Štikonas <andrius@stikonas.eu>          *
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

#include "gui/decryptluksdialog.h"
#include "gui/decryptluksdialogwidget.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <config.h>

DecryptLuksDialog::DecryptLuksDialog(/*QWidget* parent, */const QString& deviceNode) :
// 	KDialog(parent),
	m_DialogWidget(new DecryptLuksDialogWidget(this)),
	m_DeviceNode(deviceNode)
{
	setMainWidget(&widget());
	setCaption(i18nc("@title:window", "Decrypt LUKS partition on <filename>%1</filename>", this->deviceNode()));
	setButtonText(KDialog::Ok, i18nc("@action:button", "&Decrypt"));
}
