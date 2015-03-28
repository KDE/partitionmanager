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

#if !defined(CONFIGUREOPTIONSDIALOG__H)

#define CONFIGUREOPTIONSDIALOG__H
#include "util/libpartitionmanagerguiexport.h"

#include <KConfigDialog>

class GeneralPageWidget;
class FileSystemColorsPageWidget;
class AdvancedPageWidget;
class OperationStack;

class LIBKPMGUI_EXPORT ConfigureOptionsDialog : public KConfigDialog
{
	Q_OBJECT

	public:
		ConfigureOptionsDialog(QWidget* parent, const OperationStack& ostack, const QString& name);
		~ConfigureOptionsDialog();

	protected Q_SLOTS:
		virtual void updateSettings();
		virtual void updateWidgetsDefault();
		virtual bool hasChanged();
		virtual bool isDefault();
		void onComboDefaultFileSystemActivated(int) { settingsChangedSlot(); }
		void onShredSourceActivated() { settingsChangedSlot(); }
		void onComboBackendActivated(int);

	protected:
		GeneralPageWidget& generalPageWidget() { Q_ASSERT(m_GeneralPageWidget); return *m_GeneralPageWidget; }
		FileSystemColorsPageWidget& fileSystemColorsPageWidget() { Q_ASSERT(m_FileSystemColorsPageWidget); return *m_FileSystemColorsPageWidget; }
		AdvancedPageWidget& advancedPageWidget() { Q_ASSERT(m_AdvancedPageWidget); return *m_AdvancedPageWidget; }

		const OperationStack& operationStack() const { return m_OperationStack; }

	private:
		GeneralPageWidget* m_GeneralPageWidget;
		FileSystemColorsPageWidget* m_FileSystemColorsPageWidget;
		AdvancedPageWidget* m_AdvancedPageWidget;
		const OperationStack& m_OperationStack;
};

#endif
