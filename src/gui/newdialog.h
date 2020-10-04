/*
    SPDX-FileCopyrightText: 2008-2012 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(NEWDIALOG_H)

#define NEWDIALOG_H

#include "gui/sizedialogbase.h"

#include <core/partition.h>

#include <fs/filesystem.h>

class Device;

/** Dialog to create new Partitions.

    Dialog to create a new Partition in some unallocated space on a Device.

    @author Volker Lanz <vl@fidra.de>
*/
class NewDialog : public SizeDialogBase
{
public:
    NewDialog(QWidget* parent, Device& device, Partition& unallocatedPartition, PartitionRole::Roles r);
    ~NewDialog();

protected:
    void accept() override;
    void onRoleChanged(bool);
    void onFilesystemChanged(int);
    void onLabelChanged(const QString& newLabel);

    void setupConnections() override;
    void setupDialog() override;
    void slotPasswordStatusChanged();
    void updateHideAndShow();
    void updateOkButtonStatus() override;
    void updateFileSystem(FileSystem::Type t);
    PartitionRole::Roles partitionRoles() const {
        return m_PartitionRoles;
    }
    bool canGrow() const override {
        return true;
    }
    bool canShrink() const override {
        return true;
    }
    bool canMove() const override;

    bool isValidPassword() const {
        return m_IsValidPassword;
    }

private:
    PartitionRole::Roles m_PartitionRoles;
    bool m_IsValidPassword;
};

#endif
