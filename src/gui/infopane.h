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

#if !defined(INFOPANE__H)

#define INFOPANE__H

#include <QWidget>

class Partition;
class Device;

class QGridLayout;
class QString;

/** @brief Show information about Partitions and Devices

	Child widget of the QDockWidget to show some details about the currently selected Partition
	or Device

	@author vl@fidra.de
*/
class InfoPane : public QWidget
{
	public:
		InfoPane(QWidget* parent);

	public:
		void showPartition(const Partition& p);
		void showDevice(const Device& d);
		void clear();

	protected:
		void createLabels(const QString& title, const QString& value, int y);
		int createHeader(const QString& title);
		QGridLayout& gridLayout() { Q_ASSERT(m_GridLayout); return *m_GridLayout; }

	private:
		QGridLayout* m_GridLayout;
};

#endif
