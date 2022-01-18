/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Abhijeet Sharma <sharma.abhijeet2096@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(EDITMOUNTPOINTDIALOGWIDGET_H)

#define EDITMOUNTPOINTDIALOGWIDGET_H

#include "ui_editmountpointdialogwidgetbase.h"
#include <core/fstab.h>

#include <map>

#include <QString>
#include <QStringList>
#include <QWidget>

class Partition;

class QFile;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;

class EditMountPointDialogWidget : public QWidget, public Ui::EditMountPointDialogWidgetBase
{
public:
    EditMountPointDialogWidget(QWidget* parent, Partition& p);
    ~EditMountPointDialogWidget();

    QPushButton& buttonMore() {
        return *m_ButtonMore;
    }
    QLabel& labelName() {
        return *m_LabelNameValue;
    }
    QComboBox& editPath() {
        return *m_EditPath;
    }
    QSpinBox& spinDumpFreq() {
        return *m_SpinDumpFreq;
    }
    QSpinBox& spinPassNumber() {
        return *m_SpinPassNumber;
    }
    QLabel& labelType() {
        return *m_LabelTypeValue;
    }
    QStringList options() const;
    QRadioButton& radioUUID() {
        return *m_RadioUUID;
    }
    QRadioButton& radioLabel() {
        return *m_RadioLabel;
    }
    QRadioButton& radioDeviceNode() {
        return *m_RadioDeviceNode;
    }
    FstabEntryList& fstabEntries() {
        return m_fstabEntries;
    }

    void acceptChanges();
    void removeMountPoint();
    bool writeMountpoints(const QString& filename);

protected:
    void buttonSelectClicked(bool);
    void buttonMoreClicked(bool);

private:
    void setupOptions(const QStringList& options);
    void setupRadio(const FstabEntry::Type entryType);
    std::map<QString, QCheckBox*>& boxOptions() {
        return m_BoxOptions;
    }
    const std::map<QString, QCheckBox*>& boxOptions() const {
        return m_BoxOptions;
    }

    Partition& partition() {
        return m_Partition;
    }

    const Partition& partition() const {
        return m_Partition;
    }

private:
    FstabEntryList m_fstabEntries;
    QList<FstabEntry *> entry; // All fstab entries for this partition
    FstabEntry *currentEntry;
    Partition& m_Partition;
    QString m_Options;
    QString m_deviceNode;
    QStringList mountPointList;
    std::map<QString, QCheckBox*> m_BoxOptions;
    std::map<QString, QCheckBox*>::iterator iterator_BoxOptions;
};

#endif
