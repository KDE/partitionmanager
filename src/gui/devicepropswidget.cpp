/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/devicepropswidget.h"

DevicePropsWidget::DevicePropsWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
}

void DevicePropsWidget::hideTypeRadioButtons()
{
    radioSectorBased().setVisible(false);
    radioCylinderBased().setVisible(false);
}
