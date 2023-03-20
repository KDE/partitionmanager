/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/infopane.h"

#include <core/device.h>
#include <core/diskdevice.h>
#include <core/lvmdevice.h>
#include <core/partition.h>
#include <core/softwareraid.h>

#include <fs/filesystem.h>
#include <fs/luks.h>
#include <fs/lvm2_pv.h>

#include <util/capacity.h>
#include <util/helpers.h>

#include <QFontDatabase>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLocale>
#include <QPalette>

#include <KLocalizedString>

/** Creates a new InfoPane instance
    @param parent the parent widget
*/
InfoPane::InfoPane(QWidget* parent) :
    QWidget(parent),
    m_GridLayout(new QGridLayout(this))
{
    layout()->setContentsMargins(0, 0, 0, 0);
}

/** Clears the InfoPane, leaving it empty */
void InfoPane::clear()
{
    parentWidget()->parentWidget()->setWindowTitle(xi18nc("@title:window", "Information"));
    qDeleteAll(findChildren<QLabel*>());
    qDeleteAll(findChildren<QFrame*>());
}

int InfoPane::createHeader(const QString& title, const int num_cols)
{
    int y = 0;

    QLabel* label = new QLabel(title, this);
    QFont font;
    font.setBold(true);
    font.setWeight(QFont::Bold);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    gridLayout().addWidget(label, y++, 0, 1, num_cols);

    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    gridLayout().addWidget(line, y++, 0, 1, num_cols);

    return y;
}

void InfoPane::createLabels(const QString& title, const QString& value, const int num_cols, int& x, int& y)
{
    QLabel* labelTitle = new QLabel(title, this);
    labelTitle->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    labelTitle->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    QPalette palette = labelTitle->palette();
    QColor f = palette.color(QPalette::WindowText);
    f.setAlpha(128);
    palette.setColor(QPalette::WindowText, f);
    labelTitle->setPalette(palette);

    gridLayout().addWidget(labelTitle, y, x, 1, 1);

    QLabel* labelValue = new QLabel(value, this);
    labelValue->setTextInteractionFlags(Qt::TextBrowserInteraction);
    labelValue->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    gridLayout().addWidget(labelValue, y, x + 1, 1, 1);

    x += 2;

    if (x % num_cols == 0) {
        x = 0;
        y++;
    }
}

/** Shows information about a Partition in the InfoPane
    @param area the current area the widget's dock is in
    @param p the Partition to show information about
*/
void InfoPane::showPartition(Qt::DockWidgetArea area, const Partition& p)
{
    clear();
    parentWidget()->parentWidget()->setWindowTitle(xi18nc("@title:window", "Partition Information"));

    int x = 0;
    int y = createHeader(p.deviceNode(), cols(area));
    if (p.fileSystem().type() == FileSystem::Type::Luks) { // inactive LUKS partition
        const FS::luks* luksFs = static_cast<const FS::luks*>(&p.fileSystem());
        QString deviceNode = p.partitionPath();
        createLabels(i18nc("@label partition", "File system:"), p.fileSystem().name(), cols(area), x, y);
        createLabels(i18nc("@label partition", "Capacity:"), Capacity::formatByteSize(p.capacity()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Cipher name:"), luksFs->cipherName(), cols(area), x, y);
        createLabels(i18nc("@label partition", "Cipher mode:"), luksFs->cipherMode(), cols(area), x, y);
        createLabels(i18nc("@label partition", "Hash:"), luksFs->hashName(), cols(area), x, y);
        createLabels(i18nc("@label partition", "Key size:"), QString::number(luksFs->keySize()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Payload offset:"), Capacity::formatByteSize(luksFs->payloadOffset()), cols(area), x, y);
        createLabels(i18nc("@label partition", "First sector:"), QLocale().toString(p.firstSector()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Last sector:"), QLocale().toString(p.lastSector()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Number of sectors:"), QLocale().toString(p.length()), cols(area), x, y);
    } else if (p.fileSystem().type() == FileSystem::Type::Lvm2_PV) {
        FS::lvm2_pv *lvm2PVFs;
        innerFS(&p, lvm2PVFs);
        QString deviceNode = p.partitionPath();
        createLabels(i18nc("@label partition", "File system:"), p.fileSystem().name(), cols(area), x, y);
        createLabels(i18nc("@label partition", "Capacity:"), Capacity::formatByteSize(p.capacity()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Available:"), Capacity::formatByteSize(p.available()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Used:"), Capacity::formatByteSize(p.used()), cols(area), x, y);
        createLabels(i18nc("@label partition", "PE Size:"), Capacity::formatByteSize(lvm2PVFs->peSize()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Total PE:"), QString::number(lvm2PVFs->totalPE()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Free PE:"), QString::number(lvm2PVFs->freePE()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Allocated PE:"), QString::number(lvm2PVFs->allocatedPE()), cols(area), x, y);
        createLabels(i18nc("@label partition", "First sector:"), QLocale().toString(p.firstSector()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Last sector:"), QLocale().toString(p.lastSector()), cols(area), x, y);
        createLabels(i18nc("@label partition", "Number of sectors:"), QLocale().toString(p.length()), cols(area), x, y);
    } else {
        createLabels(xi18nc("@label partition", "File system:"), p.fileSystem().name(), cols(area), x, y);
        createLabels(xi18nc("@label partition", "Capacity:"), Capacity::formatByteSize(p.capacity()), cols(area), x, y);
        createLabels(xi18nc("@label partition", "Available:"), Capacity::formatByteSize(p.available()), cols(area), x, y);
        createLabels(xi18nc("@label partition", "Used:"), Capacity::formatByteSize(p.used()), cols(area), x, y);
        createLabels(xi18nc("@label partition", "First sector:"), QLocale().toString(p.firstSector()), cols(area), x, y);
        createLabels(xi18nc("@label partition", "Last sector:"), QLocale().toString(p.lastSector()), cols(area), x, y);
        createLabels(xi18nc("@label partition", "Number of sectors:"), QLocale().toString(p.length()), cols(area), x, y);
    }
    gridLayout().setAlignment(Qt::AlignTop);
}

/** Shows information about a Device in the InfoPane
    @param area the current area the widget's dock is in
    @param d the Device to show information about
*/
void InfoPane::showDevice(Qt::DockWidgetArea area, const Device& d)
{
    clear();
    parentWidget()->parentWidget()->setWindowTitle(xi18nc("@title:window", "Device Information"));

    int x = 0;
    int y = createHeader(d.name(), cols(area));
    createLabels(xi18nc("@label device", "Path:"), d.deviceNode(), cols(area), x, y);

    QString type = QStringLiteral("---");
    QString maxPrimaries = QStringLiteral("---");

    if (d.partitionTable() != nullptr) {
        type = (d.partitionTable()->isReadOnly())
               ? xi18nc("@label device", "%1 (read only)", d.partitionTable()->typeName())
               : d.partitionTable()->typeName();
        maxPrimaries = QStringLiteral("%1/%2").arg(d.partitionTable()->numPrimaries()).arg(d.partitionTable()->maxPrimaries());
    }

    if (d.type() == Device::Type::Disk_Device) {
        const DiskDevice& disk = static_cast<const DiskDevice&>(d);

        createLabels(i18nc("@label device", "Type:"), type, cols(area), x, y);
        createLabels(i18nc("@label device", "Capacity:"), Capacity::formatByteSize(disk.capacity()), cols(area), x, y);
        createLabels(i18nc("@label device", "Total sectors:"), QLocale().toString(disk.totalSectors()), cols(area), x, y);
        createLabels(i18nc("@label device", "Logical sector size:"), Capacity::formatByteSize(disk.logicalSectorSize()), cols(area), x, y);
        createLabels(i18nc("@label device", "Physical sector size:"), Capacity::formatByteSize(disk.physicalSectorSize()), cols(area), x, y);
        createLabels(i18nc("@label device", "Primaries/Max:"), maxPrimaries, cols(area), x, y);
    } else if (d.type() == Device::Type::LVM_Device) {
        const LvmDevice& lvm = static_cast<const LvmDevice&>(d);
        createLabels(i18nc("@label device", "Volume Type:"), QStringLiteral("LVM"), cols(area), x, y);
        createLabels(i18nc("@label device", "Capacity:"), Capacity::formatByteSize(lvm.capacity()), cols(area), x, y);
        createLabels(i18nc("@label device", "PE Size:"), Capacity::formatByteSize(lvm.peSize()), cols(area), x, y);
        createLabels(i18nc("@label device", "Total PE:"),QString::number(lvm.totalPE()), cols(area), x, y);
        createLabels(i18nc("@label device", "Allocated PE:"), QString::number(lvm.allocatedPE()), cols(area), x, y);
        createLabels(i18nc("@label device", "Free PE:"), QString::number(lvm.freePE()), cols(area), x, y);
    } else if (d.type() == Device::Type::SoftwareRAID_Device) {
        const SoftwareRAID& raid = static_cast<const SoftwareRAID&>(d);
        createLabels(i18nc("@label device", "Volume Type:"), QStringLiteral("RAID"), cols(area), x, y);
        createLabels(i18nc("@label device", "Capacity:"), Capacity::formatByteSize(raid.capacity()), cols(area), x, y);
        createLabels(i18nc("@label device", "RAID Level:"), raid.raidLevel() < 0 ? QStringLiteral("---") : QString::number(raid.raidLevel()), cols(area), x, y);
        createLabels(i18nc("@label device", "Chunk Size:"),Capacity::formatByteSize(raid.chunkSize()), cols(area), x, y);
        createLabels(i18nc("@label device", "Total Chunk:"), Capacity::formatByteSize(raid.totalChunk()), cols(area), x, y);
        createLabels(i18nc("@label device", "Array Size:"), Capacity::formatByteSize(raid.arraySize()), cols(area), x, y);
    }
    gridLayout().setAlignment(Qt::AlignTop);
}

qint32 InfoPane::cols(Qt::DockWidgetArea area) const
{
    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
        return 2;

    return 6;
}
