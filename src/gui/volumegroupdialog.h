/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#if !defined(VOLUMEGROUPDIALOG__H)

#define VOLUMEGROUPDIALOG__H

#include <vector>

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

class VolumeGroupWidget;
class Partition;

class VolumeGroupDialog : public QDialog
{
    Q_DISABLE_COPY(VolumeGroupDialog)

public:
    VolumeGroupDialog(QWidget* parent, QString& vgName, std::vector<const Partition*>& pvList);
    ~VolumeGroupDialog();

protected:
    virtual void setupDialog();
    virtual void setupConstraints();
    virtual void setupConnections();

    virtual void updateOkButtonStatus();
    virtual void updateSizeInfos();
    virtual void updateSectorInfos();
    virtual void updatePartitionList();

    virtual void onVolumeTypeChanged(int index);

    VolumeGroupWidget& dialogWidget() {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }
    const VolumeGroupWidget& dialogWidget() const {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }

    QString& targetName() {
        return m_TargetName;
    }

    const QString& targetName() const {
        return m_TargetName;
    }

    std::vector<const Partition*>& targetPVList() {
        return m_TargetPVList;
    }

    const std::vector<const Partition*>& targetPVList() const {
        return m_TargetPVList;
    }

    bool isValidSize() const {
        return m_IsValidSize;
    }

    bool isValidName() const {
        return m_IsValidName;
    }

protected:
    virtual void onPartitionListChanged();

protected:
    VolumeGroupWidget* m_DialogWidget;
    QString& m_TargetName;
    std::vector<const Partition*>& m_TargetPVList;
    bool m_IsValidSize;
    bool m_IsValidName;

    qint64 m_TotalSize;
    qint64 m_TotalUsedSize;
    qint32 m_ExtentSize;

    QDialogButtonBox* dialogButtonBox;
    QPushButton* okButton;
    QPushButton* cancelButton;
    QVBoxLayout *mainLayout;
};

#endif
