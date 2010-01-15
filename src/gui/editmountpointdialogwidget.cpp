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

#include "editmountpointdialogwidget.h"
#include "editmountoptionsdialog.h"

#include <KComboBox>
#include <KLocale>
#include <KFileDialog>
#include <KLineEdit>
#include <KDebug>

#include <QString>
#include <QWidget>

EditMountPointDialogWidget::EditMountPointDialogWidget(QWidget* parent, const Partition& p) :
	QWidget(parent)
{
	setupUi(this);

#if 0
	editName().setText(fs.name());
	editName().setEnabled(fs.name().isEmpty());
	editPath().setText(fs.path());
	spinDumpFreq().setValue(fs.dumpFreq());
	spinPassNumber().setValue(fs.passNumber());

	on_m_ComboType_currentIndexChanged(fs.type());

	boxOptions()["ro"] = m_CheckReadOnly;
	boxOptions()["users"] = m_CheckUsers;
	boxOptions()["noauto"] = m_CheckNoAuto;
	boxOptions()["noatime"] = m_CheckNoAtime;
	boxOptions()["nodiratime"] = m_CheckNoDirAtime;
	boxOptions()["sync"] = m_CheckSync;
	boxOptions()["noexec"] = m_CheckNoExec;
	boxOptions()["relatime"] = m_CheckRelAtime;

	setupOptions(fs.options());
#endif
}

void EditMountPointDialogWidget::on_m_ComboType_currentIndexChanged(const QString& type)
{
	const qint32 idx = comboType().findText(type);

	if (type != i18n("other") && idx != -1)
	{
		comboType().setCurrentIndex(idx);
		editType().setEnabled(false);
	}
	else
	{
		comboType().setCurrentIndex(comboType().findText(i18n("other")));
		if (type != i18n("other"))
			editType().setText(type);
		editType().setEnabled(true);
	}
}

void EditMountPointDialogWidget::setupOptions(const QStringList& options)
{
	QStringList optTmpList;

	foreach(const QString& o, options)
		if (boxOptions().find(o) != boxOptions().end())
			boxOptions()[o]->setChecked(true);
		else
			optTmpList.append(o);

	m_Options = optTmpList.join(",");
}

void EditMountPointDialogWidget::on_m_ButtonSelect_clicked(bool)
{
	const QString s = KFileDialog::getExistingDirectory(KUrl(editPath().text()), this);
	if (!s.isEmpty())
		editPath().setText(s);
}

void EditMountPointDialogWidget::on_m_ButtonMore_clicked(bool)
{
	EditMountOptionsDialog dlg(this, m_Options.split(','));
	if (dlg.exec() == KDialog::Accepted)
		setupOptions(dlg.options());
}

QStringList EditMountPointDialogWidget::options()
{
	QStringList optList = m_Options.split(',', QString::SkipEmptyParts);

	foreach (const QString& s, boxOptions().keys())
		if (boxOptions()[s]->isChecked())
			optList.append(s);

	return optList;
}

QString EditMountPointDialogWidget::type()
{
	if (comboType().currentText() == i18n("other"))
		return editType().text();

	return comboType().currentText();
}
