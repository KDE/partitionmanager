/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/sizedetailswidget.h"

#include <limits>

SizeDetailsWidget::SizeDetailsWidget(QWidget* parent) :
    QWidget(parent),
    Ui::SizeDetailsWidgetBase()
{
    setupUi(this);

    spinFirstSector().setMaximum(std::numeric_limits<double>::max());
    spinLastSector().setMaximum(std::numeric_limits<double>::max());
}
