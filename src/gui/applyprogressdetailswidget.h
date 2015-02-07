/*************************************************************************
 *  Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(APPLYPROGRESSDETAILSWIDGET__H)

#define APPLYPROGRESSDETAILSWIDGET__H

#include "ui_applyprogressdetailswidgetbase.h"

/** Details widget for the ProgressDialog.
	@author Volker Lanz <vl@fidra.de>
*/
class ApplyProgressDetailsWidget : public QWidget, public Ui::ApplyProgressDetailsWidgetBase
{
	Q_OBJECT

	public:
		ApplyProgressDetailsWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }

	public:
		QTextEdit& editReport() { Q_ASSERT(m_EditReport); return *m_EditReport; }
		QPushButton& buttonSave() { Q_ASSERT(m_ButtonSave); return *m_ButtonSave; }
		QPushButton& buttonBrowser() { Q_ASSERT(m_ButtonBrowser); return *m_ButtonBrowser; }
};

#endif
