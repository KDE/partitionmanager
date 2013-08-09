/***************************************************************************
 *   Copyright (C) 2013 by Andrius Štikonas <andrius@stikonas.eu>          *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#if !defined(DECRYPTLUKSDIALOGWIDGET__H)

#define DECRYPTLUKSDIALOGWIDGET__H

#include "ui_decryptluksdialogwidgetbase.h"

#include <QWidget>
#include <QLabel>

#include <KLineEdit>

class DecryptLuksDialogWidget : public QWidget, public Ui::DecryptLuksDialogWidgetBase
{
	Q_OBJECT

	public:
		DecryptLuksDialogWidget(QWidget* parent);

	public:
		KLineEdit& luksName() { return *m_LineEditName; }
		const KLineEdit& luksName() const { return *m_LineEditName; }

		KLineEdit& luksPassphrase() { return *m_LineEditPassphrase; }
		const KLineEdit& luksPassphrase() const { return *m_LineEditPassphrase; }
};

#endif
