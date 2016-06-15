/*************************************************************************
 *  Copyright (C) 2009, 2010 by Volker Lanz <vl@fidra.de>                *
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

#include "gui/editmountoptionsdialog.h"
#include "gui/editmountoptionsdialogwidget.h"

#include <QDialogButtonBox>
#include <QString>
#include <QStringList>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

EditMountOptionsDialog::EditMountOptionsDialog(QWidget* parent, const QStringList& options) :
    QDialog(parent),
    m_DialogWidget(new EditMountOptionsDialogWidget(this, options))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&widget());
    setWindowTitle(i18nc("@title:window", "Edit additional mount options"));

    KConfigGroup kcg(KSharedConfig::openConfig(), "editMountOptionsDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));

    QDialogButtonBox* dbb = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                  Qt::Horizontal,
                                                  this );
    mainLayout->addWidget(dbb);
    connect(dbb, &QDialogButtonBox::accepted, this, &EditMountOptionsDialog::accept);
    connect(dbb, &QDialogButtonBox::rejected, this, &EditMountOptionsDialog::reject);
}

/** Destroys an EditMOuntOptionsDialog instance */
EditMountOptionsDialog::~EditMountOptionsDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), "editMountOptionsDialog");
    kcg.writeEntry("Geometry", saveGeometry());
}

QStringList EditMountOptionsDialog::options()
{
    QStringList rval;
    const QStringList lines = widget().editOptions().toPlainText().split(QStringLiteral("\n"));
    foreach(const QString & line, lines)
    rval.append(line.simplified().toLower());
    return rval;
}
