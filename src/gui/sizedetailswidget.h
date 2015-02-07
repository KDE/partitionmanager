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

#if !defined(SIZEDETAILSWIDGET__H)

#define SIZEDETAILSWIDGET__H

#include "ui_sizedetailswidgetbase.h"

#include <QWidget>

/** Details widget for the SizeDetailsBase
	@author Volker Lanz <vl@fidra.de>
*/
class SizeDetailsWidget : public QWidget, public Ui::SizeDetailsWidgetBase
{
	Q_OBJECT

	public:
		SizeDetailsWidget(QWidget* parent);

	public:
		QDoubleSpinBox& spinFirstSector() { Q_ASSERT(m_SpinFirstSector); return *m_SpinFirstSector; }
		const QDoubleSpinBox& spinFirstSector() const { Q_ASSERT(m_SpinFirstSector); return *m_SpinFirstSector; }

		QDoubleSpinBox& spinLastSector() { Q_ASSERT(m_SpinLastSector); return *m_SpinLastSector; }
		const QDoubleSpinBox& spinLastSector() const { Q_ASSERT(m_SpinLastSector); return *m_SpinLastSector; }

		QCheckBox& checkAlign() { Q_ASSERT(m_CheckAlign); return *m_CheckAlign; }
		const QCheckBox& checkAlign() const { Q_ASSERT(m_CheckAlign); return *m_CheckAlign; }
};

#endif
