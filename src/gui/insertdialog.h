/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(INSERTDIALOG_H)

#define INSERTDIALOG_H

#include "gui/sizedialogbase.h"

class Partition;
class Device;

/** Base class for dialogs to insert Partitions.

    Base class for dialogs that need to insert a Partition into some unallocated space on
    a Device.

    @author Volker Lanz <vl@fidra.de>
*/
class InsertDialog : public SizeDialogBase
{
public:
    InsertDialog(QWidget* parent, Device& device, Partition& insertedPartition, const Partition& destPartition);
    ~InsertDialog();

protected:
    const Partition& destPartition() const {
        return m_DestPartition;
    }
    bool canGrow() const override;
    bool canShrink() const override {
        return false;
    }

private:
    const Partition& m_DestPartition;
};

#endif
