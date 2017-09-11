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
