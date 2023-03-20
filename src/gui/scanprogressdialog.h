/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SCANPROGRESSDIALOG_H
#define SCANPROGRESSDIALOG_H

#include <QProgressDialog>

class ScanProgressDialog : public QProgressDialog
{
public:
    explicit ScanProgressDialog(QWidget* parent);

    void setProgress(int p) {
        setValue(p);
    }

    void setDeviceName(const QString& device);

protected:
    void closeEvent(QCloseEvent* e) override;
    void showEvent(QShowEvent* e) override;

};

#endif

