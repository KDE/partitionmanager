/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
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

#include "gui/infopane.h"

#include "core/device.h"
#include "core/partition.h"

#include "fs/filesystem.h"

#include "util/capacity.h"

#include <QGridLayout>
#include <QLabel>
#include <QFrame>

#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>

/** Creates a new InfoPane instance
	@param parent the parent widget
*/
InfoPane::InfoPane(QWidget* parent) :
	QWidget(parent),
	m_GridLayout(new QGridLayout(this))
{
}

/** Clears the InfoPane, leaving it empty */
void InfoPane::clear()
{
	parentWidget()->setWindowTitle(i18nc("@title:window", "Information"));
	qDeleteAll(findChildren<QWidget*>());
}

int InfoPane::createHeader(const QString& title)
{
	int y = 0;

	QLabel* label = new QLabel(title, this);
	QFont font;
	font.setBold(true);
	font.setWeight(75);
	label->setFont(font);
	label->setAlignment(Qt::AlignCenter);
	gridLayout().addWidget(label, y++, 0, 1, 2);

	QFrame* line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	gridLayout().addWidget(line, y++, 0, 1, 2);

	return y;
}

void InfoPane::createLabels(const QString& title, const QString& value, int y)
{
	QLabel* labelTitle = new QLabel(title, this);
	labelTitle->setFont(KGlobalSettings::smallestReadableFont());
	labelTitle->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

	QPalette palette = labelTitle->palette();
	QColor f = palette.color(QPalette::Foreground);
	f.setAlpha(128);
	palette.setColor(QPalette::Foreground, f);
	labelTitle->setPalette(palette);

	gridLayout().addWidget(labelTitle, y, 0, 1, 1);

	QLabel* labelValue = new QLabel(value, this);
	labelValue->setFont(KGlobalSettings::smallestReadableFont());
	gridLayout().addWidget(labelValue, y, 1, 1, 1);
}

/** Shows information about a Partition in the InfoPane
	@param p the Partition to show information about
*/
void InfoPane::showPartition(const Partition& p)
{
	clear();
	parentWidget()->setWindowTitle(i18nc("@title:window", "Partition Information"));

	int y = createHeader(p.deviceNode());
	createLabels(i18nc("@label partition", "File system:"), p.fileSystem().name(), y++);
	createLabels(i18nc("@label partition", "Capacity:"), Capacity(p).toString(), y++);
	createLabels(i18nc("@label partition", "Available:"), Capacity(p, Capacity::Available).toString(), y++);
	createLabels(i18nc("@label partition", "Used:"), Capacity(p, Capacity::Used).toString(), y++);
	createLabels(i18nc("@label partition", "First sector:"), KGlobal::locale()->formatNumber(p.firstSector(), 0), y++);
	createLabels(i18nc("@label partition", "Last sector:"), KGlobal::locale()->formatNumber(p.lastSector(), 0), y++);
	createLabels(i18nc("@label partition", "Number of sectors:"), KGlobal::locale()->formatNumber(p.length(), 0), y++);
}

/** Shows information about a Device in the InfoPane
	@param d the Device to show information about
*/
void InfoPane::showDevice(const Device& d)
{
	clear();
	parentWidget()->setWindowTitle(i18nc("@title:window", "Device Information"));

	int y = createHeader(d.name());
	createLabels(i18nc("@label device", "Path:"), d.deviceNode(), y++);

	QString type = "---";
	QString maxPrimaries = "---";

	if (d.partitionTable() != NULL)
	{
		type = (d.partitionTable()->isReadOnly())
			? i18nc("@label device", "%1 (read only)", d.partitionTable()->typeName())
			: d.partitionTable()->typeName();
		maxPrimaries = QString("%1/%2").arg(d.partitionTable()->numPrimaries()).arg(d.partitionTable()->maxPrimaries());
	}

	createLabels(i18nc("@label device", "Type:"), type, y++);
	createLabels(i18nc("@label device", "Capacity:"), Capacity(d).toString(), y++);
	createLabels(i18nc("@label device", "Total sectors:"), KGlobal::locale()->formatNumber(d.totalSectors(), 0), y++);
	createLabels(i18nc("@label device", "Heads:"), QString::number(d.heads()), y++);
	createLabels(i18nc("@label device", "Cylinders:"), KGlobal::locale()->formatNumber(d.cylinders(), 0), y++);
	createLabels(i18nc("@label device", "Sectors:"), KGlobal::locale()->formatNumber(d.sectorsPerTrack(), 0), y++);
	createLabels(i18nc("@label device", "Sector size:"), Capacity(d.sectorSize()).toString(Capacity::Byte, Capacity::AppendUnit), y++);
	createLabels(i18nc("@label device", "Cylinder size:"), i18ncp("@label", "1 Sector", "%1 Sectors", d.cylinderSize()), y++);
	createLabels(i18nc("@label device", "Primaries/Max:"), maxPrimaries, y++);
}
