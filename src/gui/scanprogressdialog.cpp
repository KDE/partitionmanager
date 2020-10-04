/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/scanprogressdialog.h"

#include <QCloseEvent>

#include <KLocalizedString>

ScanProgressDialog::ScanProgressDialog(QWidget* parent) :
    QProgressDialog(parent)
{
    setWindowTitle(xi18nc("@title:window", "Scanning devices..."));
    setMinimumWidth(280);
    setMinimumDuration(150);
    setValue(0);
    setAttribute(Qt::WA_ShowModal, true);
}

void ScanProgressDialog::closeEvent(QCloseEvent* e)
{
    e->ignore();
}

void ScanProgressDialog::setDeviceName(const QString& d)
{
    if (d.isEmpty())
        setLabelText(xi18nc("@label", "Scanning..."));
    else
        setLabelText(xi18nc("@label", "Scanning device: <filename>%1</filename>", d));
}

void ScanProgressDialog::showEvent(QShowEvent* e)
{
    setCancelButton(nullptr);

    QProgressDialog::showEvent(e);
}
