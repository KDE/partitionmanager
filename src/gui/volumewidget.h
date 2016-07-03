/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
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

#if !defined(VOLUMEWIDGET__H)

#define VOLUMEWIDGET__H

#include "ui_volumewidgetbase.h"

#include <QWidget>

class  VolumeWidget : public QWidget, public Ui::VolumeWidgetBase
{
public:
    VolumeWidget(QWidget* parent) :
        QWidget(parent), Ui::VolumeWidgetBase()
    {
        setupUi(this);
    }

public:

    PartTableWidget& partTableWidget() {
        Q_ASSERT(m_PartTableWidget);
        return *m_PartTableWidget;
    }

    QLineEdit& vgName() {
        Q_ASSERT(m_EditVGName);
        return *m_EditVGName;
    }

    QComboBox& volumeType() {
        Q_ASSERT(m_ComboVolumeType);
        return *m_ComboVolumeType;
    }

    QSpinBox& spinPESize() {
        Q_ASSERT(m_SpinPESize);
        return *m_SpinPESize;

    }
    ListPhysicalVolumes& listPV() {
        Q_ASSERT(m_ListPV);
        return *m_ListPV;
    }

    QLabel& totalSize() {
        Q_ASSERT(m_LabelTotalSize);
        return *m_LabelTotalSize;
    }

    QLabel& totalSectors() {
        Q_ASSERT(m_LabelTotalSectors);
        return *m_LabelTotalSectors;
    }

    QLabel& totalLV() {
        Q_ASSERT(m_LabelTotalLV);
        return *m_LabelTotalLV;
    }

    QLabel& textVGName() {
        Q_ASSERT(m_LabelTextVGName);
        return *m_LabelTextVGName;
    }

    QLabel& textVolumeType() {
        Q_ASSERT(m_LabelTextVolumeType);
        return *m_LabelTextVolumeType;
    }

    QLabel& textTotalSize() {
        Q_ASSERT(m_LabelTextTotalSize);
        return *m_LabelTextTotalSize;
    }

    QLabel& textTotalSectors() {
        Q_ASSERT(m_LabelTextTotalSectors);
        return *m_LabelTextTotalSectors;
    }

    QLabel& textTotalLV() {
        Q_ASSERT(m_LabelTextTotalLV);
        return *m_LabelTextTotalLV;
    }

    QLabel& textTotalPESize() {
        Q_ASSERT(m_LabelTextPESize);
        return *m_LabelTextPESize;
    }

};

#endif
