/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(FORMATTEDSPINBOX_H)

#define FORMATTEDSPINBOX_H

#include <QDoubleSpinBox>

class FormattedSpinBox : public QDoubleSpinBox
{
public:
    explicit FormattedSpinBox(QWidget* parent = nullptr) : QDoubleSpinBox(parent) {}

public:
    QString textFromValue(double value) const override;
    double valueFromText(const QString& text) const override;

private:
    QString stripped(const QString &t, int *pos = nullptr) const;
};

#endif
