/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(VOLUMEGROUPWIDGET_H)

#define VOLUMEGROUPWIDGET_H

#include "ui_volumegroupwidgetbase.h"

#include <QWidget>

class  VolumeGroupWidget : public QWidget, public Ui::VolumeGroupWidgetBase
{
public:
    explicit VolumeGroupWidget(QWidget* parent) :
        QWidget(parent), Ui::VolumeGroupWidgetBase()
    {
        setupUi(this);
    }

public:

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

    QLabel& totalUsedSize() {
        Q_ASSERT(m_LabelTotalUsedSize);
        return *m_LabelTotalUsedSize;
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

    QLabel& textTotalUsedSize() {
        Q_ASSERT(m_LabelTextTotalUsedSize);
        return *m_LabelTextTotalUsedSize;
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
