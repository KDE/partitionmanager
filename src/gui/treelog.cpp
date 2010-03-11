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
#include "util/helpers.h"

#include <kactioncollection.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>

#include <QFile>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDateTime>
#include <QTextStream>

#include <config.h>

/** Creates a new TreeLog instance.
	@param parent the parent widget
*/
TreeLog::TreeLog(QWidget* parent) :
	QWidget(parent),
	Ui::TreeLogBase()
{
	setupUi(this);

	treeLog().header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(treeLog().header(), SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(onHeaderContextMenu(const QPoint&)));

}

TreeLog::~TreeLog()
{
	saveConfig();
}

void TreeLog::init()
{
	loadConfig();
}

void TreeLog::loadConfig()
{
	QList<int> colWidths = Config::treeLogColumnWidths();
	QList<int> colPositions = Config::treeLogColumnPositions();
	QList<int> colVisible = Config::treeLogColumnVisible();
	QHeaderView* header = treeLog().header();

	for (int i = 0; i < treeLog().columnCount(); i++)
	{
		if (colPositions[0] != -1 && colPositions.size() > i)
			header->moveSection(header->visualIndex(i), colPositions[i]);

		if (colVisible[0] != -1 && colVisible.size() > i)
			treeLog().setColumnHidden(i, colVisible[i] == 0);

		if (colWidths[0] != -1 && colWidths.size() > i)
			treeLog().setColumnWidth(i, colWidths[i]);
	}
}

void TreeLog::saveConfig() const
{
	kDebug() << "saving config";

	QList<int> colWidths;
	QList<int> colPositions;
	QList<int> colVisible;

	for (int i = 0; i < treeLog().columnCount(); i++)
	{
		colPositions.append(treeLog().header()->visualIndex(i));
		colVisible.append(treeLog().isColumnHidden(i) ? 0 : 1);
		colWidths.append(treeLog().columnWidth(i));
	}

	Config::setTreeLogColumnPositions(colPositions);
	Config::setTreeLogColumnVisible(colVisible);
	Config::setTreeLogColumnWidths(colWidths);

	Config::self()->writeConfig();
}

void TreeLog::onHeaderContextMenu(const QPoint& pos)
{
	showColumnsContextMenu(pos, treeLog());
}

void TreeLog::onClearLog()
{
	while (QTreeWidgetItem* item = treeLog().takeTopLevelItem(0))
		delete item;
}

void TreeLog::onSaveLog()
{
	const QString fileName = KFileDialog::getSaveFileName(KUrl("kfiledialog://saveLog"));

	if (!fileName.isEmpty())
	{
		QFile file(fileName);

		if (QFile::exists(fileName) && KMessageBox::warningContinueCancel(this, i18nc("@info", "Do you want to overwrite the existing file <filename>%1</filename>?", fileName), i18nc("@title:window", "Overwrite Existing File?"), KGuiItem(i18nc("@action:button", "Overwrite File")), KStandardGuiItem::cancel()) != KMessageBox::Continue)
			return;

		if (!file.open(QFile::WriteOnly | QFile::Truncate))
		{
			KMessageBox::error(this, i18nc("@info", "Could not create output file <filename>%1</filename>.", fileName), i18nc("@title:window", "Error Saving Log File"));
			return;
		}

		QTextStream stream(&file);

		for (qint32 idx = 0; idx < treeLog().topLevelItemCount(); idx++)
		{
			QTreeWidgetItem* item = treeLog().topLevelItem(idx);

			stream << item->text(1) << ": " << item->text(2) << "\n";
		}
	}
}

void TreeLog::on_m_TreeLog_customContextMenuRequested(const QPoint& pos)
{
	emit contextMenuRequested(treeLog().viewport()->mapToGlobal(pos));
}

void TreeLog::onNewLogMessage(Log::Level logLevel, const QString& s)
{
	static const char* icons[] =
	{
		"tools-report-bug",
		"dialog-information",
		"dialog-warning",
		"dialog-error"
	};

	kDebug() << s;

	if (logLevel >= Config::minLogLevel())
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();

		item->setIcon(0, SmallIcon(icons[logLevel]));
		item->setText(1, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		item->setText(2, s);

		treeLog().addTopLevelItem(item);
		treeLog().scrollToBottom();
	}
}
