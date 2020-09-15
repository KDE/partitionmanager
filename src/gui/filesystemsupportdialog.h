/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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
