/***************************************************************************
 *   Copyright (C) 2009 by Volker Lanz <vl@fidra.de>                       *
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

#if !defined(MOUNTMANEDITDIALOGWIDGET__H)

#define MOUNTMANEDITDIALOGWIDGET__H

#include "ui_editmountpointdialogwidgetbase.h"

#include <QMap>
#include <QString>

class Partition;

class KPushButton;
class KLineEdit;
class KComboBox;

class QWidget;
class QSpinBox;
class QCheckBox;
class QStringList;

class EditMountPointDialogWidget : public QWidget, public Ui::EditMountPointDialogWidgetBase
{
	Q_OBJECT

	public:
		EditMountPointDialogWidget(QWidget* parent, const Partition& p);

	public:
		KPushButton& buttonMore() { return *m_ButtonMore; }
		KLineEdit& editName() { return *m_EditName; }
		KLineEdit& editPath() { return *m_EditPath; }
		QSpinBox& spinDumpFreq() { return *m_SpinDumpFreq; }
		QSpinBox& spinPassNumber() { return *m_SpinPassNumber; }
		KLineEdit& editType() { return *m_EditType; }
		KComboBox& comboType() { return *m_ComboType; }

		QStringList options();
		QString type();

	protected slots:
		void on_m_ComboType_currentIndexChanged(const QString& type);
		void on_m_ButtonSelect_clicked(bool);
		void on_m_ButtonMore_clicked(bool);

	private:
		void setupOptions(const QStringList& options);
		QMap<QString, QCheckBox*>& boxOptions() { return m_BoxOptions; }
		const QMap<QString, QCheckBox*>& boxOptions() const { return m_BoxOptions; }

	private:
		QString m_Options;
		QMap<QString, QCheckBox*> m_BoxOptions;
};

#endif
