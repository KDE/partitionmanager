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

#include "gui/treelog.h"

#include "gui/partitionmanagerwidget.h"

#include "util/globallog.h"

#include <kactioncollection.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <QTreeWidget>
#include <QDateTime>

/** Creates a new TreeLog instance.
	@param parent the parent widget
*/
TreeLog::TreeLog(QWidget* parent) :
	QWidget(parent),
	Ui::TreeLogBase(),
	m_ActionCollection(NULL),
	m_PartitionManagerWidget(NULL)
{
	setupUi(this);
}

void TreeLog::onNewLogMessage(log::Level logLevel, const QString& s)
{
	static const char* icons[] =
	{
		"tools-report-bug",
		"dialog-information",
		"dialog-warning",
		"dialog-error"
	};

	kDebug() << s;

	QTreeWidgetItem* item = new QTreeWidgetItem();

	item->setIcon(0, SmallIcon(icons[logLevel]));
	item->setText(0, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	item->setText(1, s);

	treeLog().addTopLevelItem(item);

	for (int i = 0; i < treeLog().model()->columnCount(); i++)
		treeLog().resizeColumnToContents(i);

	treeLog().scrollToBottom();
}
