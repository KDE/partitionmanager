/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/createpartitiontabledialog.h"
#include "gui/createpartitiontablewidget.h"

#include <core/device.h>

#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>
#include <KMessageBox>

#include <config.h>

CreatePartitionTableDialog::CreatePartitionTableDialog(QWidget* parent, const Device& d) :
    QDialog(parent),
    m_DialogWidget(new CreatePartitionTableWidget(this)),
    m_Device(d)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&widget());
    setWindowTitle(xi18nc("@title:window", "Create a New Partition Table on <filename>%1</filename>", device().deviceNode()));
    dialogButtonBox = new QDialogButtonBox;
    createButton = dialogButtonBox->addButton(QDialogButtonBox::Ok);
    createButton->setText(xi18nc("@action:button", "Create &New Partition Table"));
    cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
    mainLayout->addWidget(dialogButtonBox);

    connect(&widget().radioMSDOS(), &QRadioButton::toggled, this, &CreatePartitionTableDialog::onMSDOSToggled);
    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &CreatePartitionTableDialog::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &CreatePartitionTableDialog::reject);
}

PartitionTable::TableType CreatePartitionTableDialog::type() const
{
    if (widget().radioGPT().isChecked())
        return PartitionTable::gpt;

    return PartitionTable::msdos;
}

void CreatePartitionTableDialog::onMSDOSToggled(bool on)
{
    if (on && device().totalLogical() > 0xffffffff) {
        if (KMessageBox::warningContinueCancel(this,
                                               xi18nc("@info",
                                                       "<para>Do you really want to create an MS-Dos partition table on <filename>%1</filename>?</para>"
                                                       "<para>This device has more than 2^32 sectors. That is the most the MS-Dos partition table type supports, so you will not be able to use the whole device.</para>", device().deviceNode()),
                                               xi18nc("@title:window", "Really Create MS-Dos Partition Table Type?"),
                                               KGuiItem(xi18nc("@action:button", "Create MS-Dos Type"), QStringLiteral("arrow-right")),
                                               KStandardGuiItem::cancel(), QStringLiteral("reallyCreateMSDOSOnLargeDevice")) == KMessageBox::Cancel) {
            widget().radioGPT().setChecked(true);
        }
    }
}

