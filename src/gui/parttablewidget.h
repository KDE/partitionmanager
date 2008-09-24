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

#if !defined(PARTTABLEWIDGET__H)

#define PARTTABLEWIDGET__H

#include "gui/partwidgetbase.h"

#include <QWidget>
#include <QList>

class PartWidget;
class PartitionTable;

class QPaintEvent;
class QResizeEvent;
class QMouseEvent;

/** @brief Widget that represents a PartitionTable.
	@author vl@fidra.de
*/
class PartTableWidget : public QWidget, public PartWidgetBase
{
	Q_OBJECT

	public:
		PartTableWidget(QWidget* parent);

	public:
		void setPartitionTable(const PartitionTable* ptable);
		
		PartWidget* activeWidget() { return m_ActiveWidget; } /**< @return the active widget or NULL if none */
		const PartWidget* activeWidget() const { return m_ActiveWidget; } /**< @return the active widget or NULL if none */ 
		
		void setActiveWidget(PartWidget* partWidget);
		void setActivePartition(const Partition* p);
		void clear();

	signals:
		void itemSelectionChanged(PartWidget*);
		void itemActivated(const PartWidget*);

	protected:
		void resizeEvent(QResizeEvent* event);
		void paintEvent(QPaintEvent* event);
		void mousePressEvent(QMouseEvent* event);
		void mouseDoubleClickEvent(QMouseEvent* event);
		
		void drawPartitions() const;
		
		QList<PartWidget*>& widgets() { return m_Widgets; }
		const QList<PartWidget*>& widgets() const { return m_Widgets; }

		const PartitionTable* partitionTable() const { return m_PartitionTable; }
		
	private:
		const PartitionTable* m_PartitionTable;
		QList<PartWidget*> m_Widgets;
		PartWidget* m_ActiveWidget;
};

#endif

