/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/formattedspinbox.h"

#include <QLocale>

// private method from Qt sources, qabstractspinbox.h:

QString FormattedSpinBox::stripped(const QString &t, int *pos) const
{
    QString text = t;
    if (specialValueText().size() == 0 || text != specialValueText()) {
        int from = 0;
        int size = text.size();
        bool changed = false;
        if (prefix().size() && text.startsWith(prefix())) {
            from += prefix().size();
            size -= from;
            changed = true;
        }
        if (suffix().size() && text.endsWith(suffix())) {
            size -= suffix().size();
            changed = true;
        }
        if (changed)
            text = text.mid(from, size);
    }

    const int s = text.size();
    text = text.trimmed();
    if (pos)
        (*pos) -= (s - text.size());
    return text;
}

QString FormattedSpinBox::textFromValue(double value) const
{
    return QLocale().toString(value, 'f', decimals());
}

double FormattedSpinBox::valueFromText(const QString& text) const
{
    return QLocale().toDouble(stripped(text));
}
