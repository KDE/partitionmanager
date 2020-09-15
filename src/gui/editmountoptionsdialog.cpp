/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/editmountoptionsdialog.h"
#include "gui/editmountoptionsdialogwidget.h"

#include <QDialogButtonBox>
#include <QStringList>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

EditMountOptionsDialog::EditMountOptionsDialog(QWidget* parent, const QStringList& options) :
    QDialog(parent),
    m_DialogWidget(new EditMountOptionsDialogWidget(this, options))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&widget());
    setWindowTitle(xi18nc("@title:window", "Edit additional mount options"));

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
    const QStringList lines = widget().editOptions().toPlainText().split(QLatin1Char('\n'));
    for (const auto &line : lines)
        rval.append(line.simplified().toLower());
    return rval;
}
