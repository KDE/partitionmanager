/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/scanprogressdialog.h"
#include <KLocalizedString>
#include <QCloseEvent>
#include <QProgressBar>

ScanProgressDialog::ScanProgressDialog(QWidget* parent) :
    QProgressDialog(parent)
{
    setWindowTitle(xi18nc("@title:window", "Scanning Devices…"));
    setMinimumWidth(280);
    setMinimumDuration(150);
    setValue(0);
    setAttribute(Qt::WA_ShowModal, true);

    QProgressBar* progressBar = findProgressBar();
    if (progressBar) {
        progressBar->setFormat(i18nc("%p is the percent value, % is the percent sign", "%p%"));
    }
}

void ScanProgressDialog::closeEvent(QCloseEvent* e)
{
    e->ignore();
}

void ScanProgressDialog::setDeviceName(const QString& device)
{
    if (device.isEmpty()) {
        setLabelText(xi18nc("@label", "Scanning…"));
    } else {
        setLabelText(xi18nc("@label", "Scanning device: <filename>%1</filename>", device));
    }
}

void ScanProgressDialog::showEvent(QShowEvent* e)
{
    setCancelButton(nullptr);
    QProgressDialog::showEvent(e);
}

QProgressBar* ScanProgressDialog::findProgressBar()
{
    auto children = findChildren<QProgressBar*>();
    return children.isEmpty() ? nullptr : children.first();
}
