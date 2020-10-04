/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(EDITMOUNTOPTIONSDIALOG_H)

#define EDITMOUNTOPTIONSDIALOG_H

#include <QDialog>

class EditMountOptionsDialogWidget;
class QStringList;
class QString;
class QWidget;

class EditMountOptionsDialog : public QDialog
{
public:
    EditMountOptionsDialog(QWidget* parent, const QStringList& options);
    ~EditMountOptionsDialog();

public:
    QStringList options();

protected:
    EditMountOptionsDialogWidget& widget() {
        return *m_DialogWidget;
    }

private:
    EditMountOptionsDialogWidget* m_DialogWidget;
};

#endif

