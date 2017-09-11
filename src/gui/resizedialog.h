/*************************************************************************
 *  Copyright (C) 2008, 2010, 2012 by Volker Lanz <vl@fidra.de>          *
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

#if !defined(RESIZEDIALOG_H)

#define RESIZEDIALOG_H

#include "gui/sizedialogbase.h"

class Partition;
class Device;

/** Let the user resize or move a Partition.

    @author Volker Lanz <vl@fidra.de>
*/
class ResizeDialog : public SizeDialogBase
{
public:
    ResizeDialog(QWidget* parent, Device& device, Partition& p, qint64 minFirst, qint64 maxLast);
    ~ResizeDialog();

public:
    bool isModified() const;
    qint64 resizedFirstSector() const {
        return m_ResizedFirstSector;
    }
    qint64 resizedLastSector() const {
        return m_ResizedLastSector;
    }

    void accept() override;
    void reject() override;

protected:
    bool canGrow() const override;
    bool canShrink() const override;
    bool canMove() const override;
    void setupDialog() override;
    void setDirty() override;
    void rollback();
    void setResizedFirstSector(qint64 s) {
        m_ResizedFirstSector = s;
    }
    void setResizedLastSector(qint64 s) {
        m_ResizedLastSector = s;
    }

    qint64 originalFirstSector() const {
        return m_OriginalFirstSector;
    }
    qint64 originalLastSector() const {
        return m_OriginalLastSector;
    }

private:
    qint64 m_OriginalFirstSector;
    qint64 m_OriginalLastSector;
    qint64 m_ResizedFirstSector;
    qint64 m_ResizedLastSector;
};

#endif
