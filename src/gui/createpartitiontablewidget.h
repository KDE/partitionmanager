/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#if !defined(CREATEPARTITIONTABLEWIDGET__H)

#define CREATEPARTITIONTABLEWIDGET__H

#include "ui_createpartitiontablewidgetbase.h"

#include <QWidget>
#include <QRadioButton>
#include <QLabel>

class CreatePartitionTableWidget : public QWidget, public Ui::CreatePartitionTableWidgetBase
{
	Q_OBJECT

	public:
		CreatePartitionTableWidget(QWidget* parent);

	public:
		QRadioButton& radioMSDOS() { return *m_RadioMSDOS; }
		const QRadioButton& radioMSDOS() const { return *m_RadioMSDOS; }

		QRadioButton& radioGPT() { return *m_RadioGPT; }
		const QRadioButton& radioGPT() const { return *m_RadioGPT; }

		QLabel& iconLabel() { return *m_IconLabel; }
};

#endif
