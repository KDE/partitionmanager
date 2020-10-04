/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/insertdialog.h"
#include "gui/sizedialogwidget.h"
#include "gui/sizedetailswidget.h"

#include <core/partition.h>

#include <fs/filesystem.h>

#include <ops/resizeoperation.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/** Creates a new InsertDialog instance.
    @param parent the parent widget
    @param device the Device the Partition to insert is on
    @param insertedPartition the Partition to insert
    @param destpartition the Partition the new one is to be inserted to
*/
InsertDialog::InsertDialog(QWidget* parent, Device& device, Partition& insertedPartition, const Partition& destpartition) :
    SizeDialogBase(parent, device, insertedPartition, destpartition.firstSector(), destpartition.lastSector()),
    m_DestPartition(destpartition)
{
    setWindowTitle(xi18nc("@title:window", "Insert a partition"));

    partition().move(destPartition().firstSector());
    partition().fileSystem().move(destPartition().fileSystem().firstSector());

    dialogWidget().hideRole();
    dialogWidget().hideFileSystem();
    dialogWidget().hideLabel();

    setupDialog();
    setupConstraints();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), "insertDialog");
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys an InsertDialog instance */
InsertDialog::~InsertDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), "insertDialog");
    kcg.writeEntry("Geometry", saveGeometry());
}

bool InsertDialog::canGrow() const
{
    return ResizeOperation::canGrow(&partition());
}
