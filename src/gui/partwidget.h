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

#if !defined(PARTWIDGET__H)

#define PARTWIDGET__H

#include "gui/partwidgetbase.h"
#include "gui/parttablewidget.h"

#include "core/partition.h"

#include <QWidget>
#include <QPointer>

class QPaintEvent;
class QResizeEvent;

/** @brief Widget that represents a Partition.

	Represents a single Partition (possibly with its children, in case of an extended Partition) in the GUI.

	@author vl@fidra.de
*/
class PartWidget : public QWidget, public PartWidgetBase
{
	Q_OBJECT

	public:
		PartWidget(QWidget* parent, const PartTableWidget* ptWidget, const Partition* p, bool showChildren = true);

	public:
		bool active() const;
		void updateChildren();

		const Partition* partition() const { return m_Partition.isNull() ? NULL : m_Partition; } /**< @return the widget's Partition or NULL if none set */

	protected:
		void paintEvent(QPaintEvent* event);
		void resizeEvent(QResizeEvent* event);

		PartTableWidget* partTableWidget() { return m_PartTableWidget.isNull() ? NULL : m_PartTableWidget; }
		const PartTableWidget* partTableWidget() const { return m_PartTableWidget.isNull() ? NULL : m_PartTableWidget; }

		QList<PartWidget*>& widgets() { return m_Widgets; }
		const QList<PartWidget*>& widgets() const { return m_Widgets; }

		void drawPartition(QWidget* destWidget);
		bool showChildren() const { return m_ShowChildren; }

		QColor activeColor(const QColor& col) const;

	private:
		QPointer<PartTableWidget> m_PartTableWidget;
		QPointer<Partition> m_Partition;
		QList<PartWidget*> m_Widgets;
		bool m_ShowChildren;
};

#endif

