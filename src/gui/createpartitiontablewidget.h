/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(CREATEPARTITIONTABLEWIDGET_H)

#define CREATEPARTITIONTABLEWIDGET_H

#include "ui_createpartitiontablewidgetbase.h"

#include <QWidget>
#include <QRadioButton>
#include <QLabel>

class CreatePartitionTableWidget : public QWidget, public Ui::CreatePartitionTableWidgetBase
{
public:
    explicit CreatePartitionTableWidget(QWidget* parent);

public:
    QRadioButton& radioMSDOS() {
        return *m_RadioMSDOS;
    }
    const QRadioButton& radioMSDOS() const {
        return *m_RadioMSDOS;
    }

    QRadioButton& radioGPT() {
        return *m_RadioGPT;
    }
    const QRadioButton& radioGPT() const {
        return *m_RadioGPT;
    }

    QLabel& iconLabel() {
        return *m_IconLabel;
    }
};

#endif
