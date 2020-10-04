/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(DEVICEPROPSWIDGET_H)

#define DEVICEPROPSWIDGET_H

#include "ui_devicepropswidgetbase.h"

class PartTableWidget;

/** Central widget in the DevicePropsDialog.
    @author Volker Lanz <vl@fidra.de>
*/
class DevicePropsWidget : public QWidget, public Ui::DevicePropsWidgetBase
{
public:
    explicit DevicePropsWidget(QWidget* parent);

public:
    PartTableWidget& partTableWidget() {
        Q_ASSERT(m_PartTableWidget);
        return *m_PartTableWidget;
    }

    QLabel& capacity() {
        Q_ASSERT(m_LabelCapacity);
        return *m_LabelCapacity;
    }
    QLabel& primariesMax() {
        Q_ASSERT(m_LabelPrimariesMax);
        return *m_LabelPrimariesMax;
    }
    QLabel& logicalSectorSize() {
        Q_ASSERT(m_LabelLogicalSectorSize);
        return *m_LabelLogicalSectorSize;
    }
    QLabel& physicalSectorSize() {
        Q_ASSERT(m_LabelPhysicalSectorSize);
        return *m_LabelPhysicalSectorSize;
    }
    QLabel& totalSectors() {
        Q_ASSERT(m_LabelTotalSectors);
        return *m_LabelTotalSectors;
    }
    QLabel& type() {
        Q_ASSERT(m_LabelType);
        return *m_LabelType;
    }

    QRadioButton& radioCylinderBased() {
        Q_ASSERT(m_RadioCylinderBased);
        return *m_RadioCylinderBased;
    }
    const QRadioButton& radioCylinderBased() const {
        Q_ASSERT(m_RadioCylinderBased);
        return *m_RadioCylinderBased;
    }

    QRadioButton& radioSectorBased() {
        Q_ASSERT(m_RadioSectorBased);
        return *m_RadioSectorBased;
    }
    const QRadioButton& radioSectorBased() const {
        Q_ASSERT(m_RadioSectorBased);
        return *m_RadioSectorBased;
    }

    QSpacerItem& spacerType() {
        Q_ASSERT(m_SpacerType);
        return *m_SpacerType;
    }

    QLabel& smartStatusText() {
        Q_ASSERT(m_LabelSmartStatusText);
        return *m_LabelSmartStatusText;
    }
    QLabel& smartStatusIcon() {
        Q_ASSERT(m_LabelSmartStatusIcon);
        return *m_LabelSmartStatusIcon;
    }
    QPushButton& buttonSmartMore() {
        Q_ASSERT(m_ButtonSmartMore);
        return *m_ButtonSmartMore;
    }

    void hideTypeRadioButtons();
};

#endif
