/***************************************************************************
 *   Copyright (C) 2008,2010 by Volker Lanz <vl@fidra.de>                  *
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

#include "gui/resizedialog.h"
#include "gui/sizedialogwidget.h"

#include "core/partition.h"

#include "ops/resizeoperation.h"

#include "util/capacity.h"

/** Creates a new ResizeDialog
	@param parent pointer to the parent widget
	@param device the Device the Partition to resize is on
	@param p the Partition to resize
	@param freebefore number of sectors free before the Partition to resize
	@param freeafter number of sectors free after the Partition to resize
*/
ResizeDialog::ResizeDialog(QWidget* parent, Device& device, Partition& p, qint64 minFirst, qint64 maxLast) :
	SizeDialogBase(parent, device, p, minFirst, maxLast),
	m_OriginalFirstSector(p.firstSector()),
	m_OriginalLastSector(p.lastSector())
{
	setCaption(i18nc("@title:window", "Resize/move partition: <filename>%1</filename>", partition().deviceNode()));

	dialogWidget().hideRole();
	dialogWidget().hideFileSystem();
	dialogWidget().hideLabel();

	setupConstraints();
	setupDialog();
	setupConnections();

	restoreDialogSize(KConfigGroup(KGlobal::config(), "resizeDialog"));
}

/** Destroys a ResizeDialog */
ResizeDialog::~ResizeDialog()
{
	KConfigGroup kcg(KGlobal::config(), "resizeDialog");
	saveDialogSize(kcg);
}

void ResizeDialog::setupDialog()
{
	SizeDialogBase::setupDialog();
	enableButtonOk(false);
}

void ResizeDialog::setDirty()
{
	enableButtonOk(isModified());
}

/** @return true if the user modified anything */
bool ResizeDialog::isModified() const
{
	return partition().firstSector() != originalFirstSector() || partition().lastSector() != originalLastSector();
}

bool ResizeDialog::canGrow() const
{
	return ResizeOperation::canGrow(&partition());
}

bool ResizeDialog::canShrink() const
{
	return ResizeOperation::canShrink(&partition());
}

bool ResizeDialog::canMove() const
{
	return ResizeOperation::canMove(&partition());
}
