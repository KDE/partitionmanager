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

#if !defined(PARTWIDGET__H)

#define PARTWIDGET__H

#include "gui/partwidgetbase.h"

#include <kdebug.h>

class Partition;

class QPaintEvent;
class QResizeEvent;

/** @brief Widget that represents a Partition.

	Represents a single Partition (possibly with its children, in case of an extended Partition) in the GUI.

	@author vl@fidra.de
*/
class PartWidget : public PartWidgetBase
{
	Q_OBJECT

	public:
		PartWidget(QWidget* parent, const Partition* p);

	public:
		void setActive(bool b) { m_Active = b; }
		bool isActive() const { return m_Active; } /**< @return true if this is the currently active widget */
		void updateChildren();

		const Partition& partition() const { Q_ASSERT(m_Partition); return *m_Partition; } /**< @return the widget's Partition */

	protected:
		void paintEvent(QPaintEvent* event);
		void resizeEvent(QResizeEvent* event);

		QColor activeColor(const QColor& col) const;

	private:
		const Partition* m_Partition;
		bool m_Active;
};

#endif

