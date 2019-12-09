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

#if !defined(FILESYSTEMSUPPORTDIALOG_H)

#define FILESYSTEMSUPPORTDIALOG_H

#include <QWidget>
#include <QDialog>

class QDialogButtonBox;
class QPushButton;

class FileSystemSupportDialogWidget;

/** Show supported Operations

    Dialog to show which Operations are supported for which type of FileSystem.

    @author Volker Lanz <vl@fidra.de>
*/
class FileSystemSupportDialog : public QDialog
{
    Q_DISABLE_COPY(FileSystemSupportDialog)

public:
    explicit FileSystemSupportDialog(QWidget* parent);
    ~FileSystemSupportDialog();

public:
    QSize sizeHint() const override;

protected:
    void onButtonRescanClicked();

    FileSystemSupportDialogWidget& dialogWidget() {
        Q_ASSERT(m_FileSystemSupportDialogWidget);
        return *m_FileSystemSupportDialogWidget;
    }
    const FileSystemSupportDialogWidget& dialogWidget() const {
        Q_ASSERT(m_FileSystemSupportDialogWidget);
        return *m_FileSystemSupportDialogWidget;
    }
    void setupDialog();
    void setupConnections();

private:
    FileSystemSupportDialogWidget* m_FileSystemSupportDialogWidget;
    QDialogButtonBox* dialogButtonBox;
};

#endif
