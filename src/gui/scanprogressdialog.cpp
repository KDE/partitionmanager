/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "gui/scanprogressdialog.h"

#include <KLocalizedString>

ScanProgressDialog::ScanProgressDialog(QWidget* parent) :
    QProgressDialog(parent)
{
    setWindowTitle(xi18nc("@title:window", "Scanning devices..."));
    setMinimumWidth(280);
    setMinimumDuration(150);
    setAttribute(Qt::WA_ShowModal, true);
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
    setCancelButton(0);

    QProgressDialog::showEvent(e);
}
