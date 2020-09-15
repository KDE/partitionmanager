/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(SIZEDETAILSWIDGET_H)

#define SIZEDETAILSWIDGET_H

#include "ui_sizedetailswidgetbase.h"

#include <QWidget>

/** Details widget for the SizeDetailsBase
    @author Volker Lanz <vl@fidra.de>
*/
class SizeDetailsWidget : public QWidget, public Ui::SizeDetailsWidgetBase
{
public:
    explicit SizeDetailsWidget(QWidget* parent);

public:
    QDoubleSpinBox& spinFirstSector() {
        Q_ASSERT(m_SpinFirstSector);
        return *m_SpinFirstSector;
    }
    const QDoubleSpinBox& spinFirstSector() const {
        Q_ASSERT(m_SpinFirstSector);
        return *m_SpinFirstSector;
    }

    QDoubleSpinBox& spinLastSector() {
        Q_ASSERT(m_SpinLastSector);
        return *m_SpinLastSector;
    }
    const QDoubleSpinBox& spinLastSector() const {
        Q_ASSERT(m_SpinLastSector);
        return *m_SpinLastSector;
    }

    QCheckBox& checkAlign() {
        Q_ASSERT(m_CheckAlign);
        return *m_CheckAlign;
    }
    const QCheckBox& checkAlign() const {
        Q_ASSERT(m_CheckAlign);
        return *m_CheckAlign;
    }
};

#endif
