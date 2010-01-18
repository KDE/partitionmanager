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

#if !defined(FILESYSTEMSUPPORTDIALOG__H)

#define FILESYSTEMSUPPORTDIALOG__H

#include "ui_filesystemsupportdialogwidgetbase.h"

#include <QWidget>

#include <kdialog.h>

class KPushButton;

/** @brief Show supported Operations

	Dialog to show which Operations are supported for which type of FileSystem.

	@author vl@fidra.de
*/
class FileSystemSupportDialog : public KDialog
{
	Q_OBJECT
	Q_DISABLE_COPY(FileSystemSupportDialog)

	private:
		class FileSystemSupportDialogWidget : public QWidget, public Ui::FileSystemSupportDialogWidgetBase
		{
			public:
				FileSystemSupportDialogWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }

			public:
				QTreeWidget& tree() { Q_ASSERT(m_Tree); return *m_Tree; }
				const QTreeWidget& tree() const { Q_ASSERT(m_Tree); return *m_Tree; }
				KPushButton& buttonRescan() { Q_ASSERT(m_ButtonRescan); return *m_ButtonRescan; }
				const KPushButton& buttonRescan() const { Q_ASSERT(m_ButtonRescan); return *m_ButtonRescan; }
		};

	public:
		FileSystemSupportDialog(QWidget* parent);
		~FileSystemSupportDialog();

	public:
		QSize sizeHint() const;

	protected slots:
		void onButtonRescanClicked();

	protected:
		FileSystemSupportDialogWidget& dialogWidget() { Q_ASSERT(m_FileSystemSupportDialogWidget); return *m_FileSystemSupportDialogWidget; }
		const FileSystemSupportDialogWidget& dialogWidget() const { Q_ASSERT(m_FileSystemSupportDialogWidget); return *m_FileSystemSupportDialogWidget; }
		void setupDialog();
		void setupConnections();

	private:
		FileSystemSupportDialogWidget* m_FileSystemSupportDialogWidget;
};

#endif
