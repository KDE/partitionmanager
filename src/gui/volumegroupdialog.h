/*
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(VOLUMEGROUPDIALOG_H)

#define VOLUMEGROUPDIALOG_H

#include <QVector>

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
    VolumeGroupDialog(QWidget* parent, QString& vgName, QVector<const Partition*>& pvList);
    ~VolumeGroupDialog();

protected:
    virtual void setupDialog();
    virtual void setupConstraints();
    virtual void setupConnections();

    virtual void updateOkButtonStatus();
    virtual void updateSizeInfos();
    virtual void updateSectorInfos();
    virtual void updatePartitionList();
    virtual void updateNameValidator();

    virtual void onPartitionListChanged();
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

    QVector<const Partition*>& targetPVList() {
        return m_TargetPVList;
    }

    const QVector<const Partition*>& targetPVList() const {
        return m_TargetPVList;
    }

    bool isValidSize() const {
        return m_IsValidSize;
    }

    bool isValidName() const {
        return m_IsValidName;
    }

private:
    void updateComponents();

protected:
    VolumeGroupWidget* m_DialogWidget;
    QString& m_TargetName;
    QVector<const Partition*>& m_TargetPVList;
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
