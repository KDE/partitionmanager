/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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
