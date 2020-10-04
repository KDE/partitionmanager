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

    QLineEdit& vgName() const {
        Q_ASSERT(m_EditVGName);
        return *m_EditVGName;
    }

    QComboBox& volumeType() const {
        Q_ASSERT(m_ComboVolumeType);
        return *m_ComboVolumeType;
    }

    QSpinBox& spinPESize() const {
        Q_ASSERT(m_SpinPESize);
        return *m_SpinPESize;
    }

    ListPhysicalVolumes& listPV() const {
        Q_ASSERT(m_ListPV);
        return *m_ListPV;
    }

    QLabel& totalSize() const {
        Q_ASSERT(m_LabelTotalSize);
        return *m_LabelTotalSize;
    }

    QLabel& totalSectors() const {
        Q_ASSERT(m_LabelTotalSectors);
        return *m_LabelTotalSectors;
    }

    QLabel& totalUsedSize() const {
        Q_ASSERT(m_LabelTotalUsedSize);
        return *m_LabelTotalUsedSize;
    }

    QLabel& totalLV() const {
        Q_ASSERT(m_LabelTotalLV);
        return *m_LabelTotalLV;
    }

    QLabel& textVGName() const {
        Q_ASSERT(m_LabelTextVGName);
        return *m_LabelTextVGName;
    }

    QLabel& textVolumeType() const {
        Q_ASSERT(m_LabelTextVolumeType);
        return *m_LabelTextVolumeType;
    }

    QLabel& textTotalSize() const {
        Q_ASSERT(m_LabelTextTotalSize);
        return *m_LabelTextTotalSize;
    }

    QLabel& textTotalSectors() const {
        Q_ASSERT(m_LabelTextTotalSectors);
        return *m_LabelTextTotalSectors;
    }

    QLabel& textTotalUsedSize() const {
        Q_ASSERT(m_LabelTextTotalUsedSize);
        return *m_LabelTextTotalUsedSize;
    }

    QLabel& textTotalLV() const {
        Q_ASSERT(m_LabelTextTotalLV);
        return *m_LabelTextTotalLV;
    }

    QLabel& textTotalPESize() const {
        Q_ASSERT(m_LabelTextPESize);
        return *m_LabelTextPESize;
    }

    QSpinBox& chunkSize() const {
        Q_ASSERT(m_ChunkSize);
        return *m_ChunkSize;
    }

    QComboBox& raidLevel() const {
        Q_ASSERT(m_RaidLevel);
        return *m_RaidLevel;
    }

    QLabel& textChunkSize() const {
        Q_ASSERT(m_LabelChunkSize);
        return *m_LabelChunkSize;
    }

    QLabel& textRaidLevel() const {
        Q_ASSERT(m_LabelRaidLevel);
        return *m_LabelRaidLevel;
    }

};

#endif
