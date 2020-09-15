/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(CREATEPARTITIONTABLEDIALOG_H)

#define CREATEPARTITIONTABLEDIALOG_H

#include "gui/createpartitiontablewidget.h"

#include <core/partitiontable.h>

#include <QDialog>

class Device;
class QDialogButtonBox;
class QPushButton;

class CreatePartitionTableDialog : public QDialog
{
public:
    CreatePartitionTableDialog(QWidget* parent, const Device& d);

public:
    PartitionTable::TableType type() const;

protected:
    CreatePartitionTableWidget& widget() {
        return *m_DialogWidget;
    }
    const CreatePartitionTableWidget& widget() const {
        return *m_DialogWidget;
    }
    const Device& device() const {
        return m_Device;
    }

    void onMSDOSToggled(bool on);

private:
    CreatePartitionTableWidget* m_DialogWidget;
    const Device& m_Device;

    QDialogButtonBox* dialogButtonBox;
    QPushButton* createButton;
    QPushButton* cancelButton;
};


#endif
