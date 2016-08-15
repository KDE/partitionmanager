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

#if !defined(VOLUMEDIALOG__H)

#define VOLUMEDIALOG__H

#include <QDialog>

class VolumeWidget;
class Partition;

class QDialogButtonBox;
class QPushButton;
class QVBoxLayout;
class QWidget;
class QString;

class VolumeDialog : public QDialog
{
    Q_DISABLE_COPY(VolumeDialog)

public:
    VolumeDialog(QWidget* parent, QString& vgname, QStringList& pvlist);
    ~VolumeDialog();

protected:
    virtual void setupDialog();
    virtual void setupConstraints();
    virtual void setupConnections();

    virtual void updateOkButtonStatus();
    virtual void updatePartTable();
    virtual void updateSizeInfos();
    virtual void updatePartitionList();

    virtual void onVolumeTypeChanged(int index);

    VolumeWidget& dialogWidget() {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }
    const VolumeWidget& dialogWidget() const {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }

    QString& targetName() {
        return m_TargetName;
    }

    const QString& targetName() const {
        return m_TargetName;
    }

    QStringList& targetPVList() {
        return m_TargetPVList;
    }

    const QStringList& targetPVList() const {
        return m_TargetPVList;
    }

    bool isValidSize() const {
        return m_IsValidSize;
    }

protected:
    virtual void onPartitionListChanged();

protected:
    VolumeWidget* m_DialogWidget;
    QString& m_TargetName;
    QStringList& m_TargetPVList;
    bool m_IsValidSize;

    QDialogButtonBox* dialogButtonBox;
    QPushButton* okButton;
    QPushButton* cancelButton;
    QVBoxLayout *mainLayout;
};

#endif
