/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/devicepropsdialog.h"
#include "gui/devicepropswidget.h"

#include "gui/smartdialog.h"

#include <core/device.h>
#include <core/diskdevice.h>
#include <core/partitiontable.h>
#include <core/smartstatus.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QDialogButtonBox>
#include <QPointer>
#include <QPushButton>
#include <QStyle>

/** Creates a new DevicePropsDialog
    @param parent pointer to the parent widget
    @param d the Device to show properties for
*/
DevicePropsDialog::DevicePropsDialog(QWidget* parent, Device& d) :
    QDialog(parent),
    m_Device(d),
    m_DialogWidget(new DevicePropsWidget(this))
{
    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&dialogWidget());
    setWindowTitle(xi18nc("@title:window", "Device Properties: <filename>%1</filename>", device().deviceNode()));

    setupDialog();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("devicePropsDialog"));
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));

}

/** Destroys a DevicePropsDialog */
DevicePropsDialog::~DevicePropsDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("devicePropsDialog"));
    kcg.writeEntry("Geometry", saveGeometry());
}

void DevicePropsDialog::setupDialog()
{
    dialogButtonBox = new QDialogButtonBox;
    okButton = dialogButtonBox->addButton(QDialogButtonBox::Ok);
    cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
    mainLayout->addWidget(dialogButtonBox);
    okButton->setEnabled(false);
    cancelButton->setFocus();
    cancelButton->setDefault(true);
    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &DevicePropsDialog::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &DevicePropsDialog::reject);

    QString type = QStringLiteral("---");
    QString maxPrimaries = QStringLiteral("---");

    if (device().partitionTable() != nullptr) {
        type = (device().partitionTable()->isReadOnly())
               ? xi18nc("@label device", "%1 (read only)", device().partitionTable()->typeName())
               : device().partitionTable()->typeName();
        maxPrimaries = QStringLiteral("%1/%2").arg(device().partitionTable()->numPrimaries()).arg(device().partitionTable()->maxPrimaries());

        dialogWidget().partTableWidget().setReadOnly(true);
        dialogWidget().partTableWidget().setPartitionTable(device().partitionTable());

        if (device().partitionTable()->type() == PartitionTable::msdos)
            dialogWidget().radioCylinderBased().setChecked(true);
        else if (device().partitionTable()->type() == PartitionTable::msdos_sectorbased)
            dialogWidget().radioSectorBased().setChecked(true);
        else
            dialogWidget().hideTypeRadioButtons();
    } else {
        dialogWidget().partTableWidget().setVisible(false);
        dialogWidget().hideTypeRadioButtons();
    }

    dialogWidget().type().setText(type);
    dialogWidget().capacity().setText(Capacity::formatByteSize(device().capacity()));
    dialogWidget().totalSectors().setText(QLocale().toString(device().totalLogical()));

    if (device().type() == Device::Type::Disk_Device) {

        const DiskDevice& disk = dynamic_cast<const DiskDevice&>(device());

        dialogWidget().primariesMax().setText(maxPrimaries);
        dialogWidget().logicalSectorSize().setText(Capacity::formatByteSize(disk.logicalSectorSize()));
        dialogWidget().physicalSectorSize().setText(Capacity::formatByteSize(disk.physicalSectorSize()));
        if (device().smartStatus().isValid()) {
            if (device().smartStatus().status()) {
                dialogWidget().smartStatusText().setText(xi18nc("@label SMART disk status", "good"));
                dialogWidget().smartStatusIcon().setPixmap(QIcon::fromTheme(QStringLiteral("dialog-ok"))
                            .pixmap(QApplication::style()->pixelMetric(QStyle::PixelMetric::PM_SmallIconSize)));

            } else {
                dialogWidget().smartStatusText().setText(xi18nc("@label SMART disk status", "BAD"));
                dialogWidget().smartStatusIcon().setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning"))
                            .pixmap(QApplication::style()->pixelMetric(QStyle::PixelMetric::PM_SmallIconSize)));
            }
        } else {
            dialogWidget().smartStatusText().setText(xi18nc("@label", "(unknown)"));
            dialogWidget().smartStatusIcon().setVisible(false);
            dialogWidget().buttonSmartMore().setVisible(false);
        }
    } else {
        if (device().type() == Device::Type::LVM_Device)
            dialogWidget().type().setText(xi18nc("@label device", "LVM Volume Group"));
        else if (device().type() == Device::Type::SoftwareRAID_Device)
            dialogWidget().type().setText(xi18nc("@label device", "Software RAID Device"));
        else
            dialogWidget().type().setText(xi18nc("@label device", "Volume Manager Device"));
        //TODO: add Volume Manger Device info
        dialogWidget().smartStatusText().setVisible(false);
        dialogWidget().smartStatusIcon().setVisible(false);
        dialogWidget().buttonSmartMore().setVisible(false);
    }

    setMinimumSize(dialogWidget().size());
    resize(dialogWidget().size());
}

void DevicePropsDialog::setupConnections()
{
    connect(&dialogWidget().radioSectorBased(), &QRadioButton::toggled, this, &DevicePropsDialog::setDirty);
    connect(&dialogWidget().radioCylinderBased(), &QRadioButton::toggled, this, &DevicePropsDialog::setDirty);
    connect(&dialogWidget().buttonSmartMore(), &QPushButton::clicked, this, &DevicePropsDialog::onButtonSmartMore);
}

void DevicePropsDialog::setDirty(bool)
{
    okButton->setEnabled(true);
    okButton->setDefault(true);
}

bool DevicePropsDialog::cylinderBasedAlignment() const
{
    return dialogWidget().radioCylinderBased().isChecked();
}

bool DevicePropsDialog::sectorBasedAlignment() const
{
    return dialogWidget().radioSectorBased().isChecked();
}

void DevicePropsDialog::onButtonSmartMore(bool)
{
    QPointer<SmartDialog> dlg = new SmartDialog(this, device());
    dlg->exec();
    delete dlg;
}
