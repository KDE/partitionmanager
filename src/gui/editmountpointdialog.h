/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Abhijeet Sharma <sharma.abhijeet2096@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(EDITMOUNTPOINTDIALOG_H)

#define EDITMOUNTPOINTDIALOG_H

#include <QDialog>

class EditMountPointDialogWidget;
class Partition;

class QWidget;
class QString;

enum class MountPointAction
{
    Remove,
    Edit
};

class EditMountPointDialog : public QDialog
{
public:
    EditMountPointDialog(QWidget* parent, Partition& p);
    ~EditMountPointDialog();

protected:
    EditMountPointDialogWidget& widget() {
        return *m_DialogWidget;
    }

    void accept_(MountPointAction action);

private:
    Partition& partition() {
        return m_Partition;
    }

private:
    Partition& m_Partition;
    EditMountPointDialogWidget* m_DialogWidget;
};

#endif

