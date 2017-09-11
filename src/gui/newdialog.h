/*************************************************************************
 *  Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                *
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
    void onLVNameChanged(const QString& newName);

    void setupConnections() override;
    void setupDialog() override;
    void slotPasswordStatusChanged();
    void updateHideAndShow();
    void updateOkButtonStatus();
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
    bool isValidLVName() const {
        return m_IsValidLVName;
    }

private:
    PartitionRole::Roles m_PartitionRoles;
    bool m_IsValidPassword;
    bool m_IsValidLVName;
};

#endif
