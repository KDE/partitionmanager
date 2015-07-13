/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(FORMATTEDSPINBOX__H)

#define FORMATTEDSPINBOX__H

#include <QDoubleSpinBox>

class FormattedSpinBox : public QDoubleSpinBox
{
public:
    FormattedSpinBox(QWidget* parent = NULL) : QDoubleSpinBox(parent) {}

public:
    virtual QString textFromValue(double value) const;
    virtual double valueFromText(const QString& text) const;

private:
    QString stripped(const QString &t, int *pos = 0) const;
};

#endif
