/*************************************************************************
 *  Copyright (C) 2009, 2010 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(EDITMOUNTPOINTDIALOGWIDGET_H)

#define EDITMOUNTPOINTDIALOGWIDGET_H

#include "ui_editmountpointdialogwidgetbase.h"
#include <core/fstab.h>

#include <map>

#include <QString>
#include <QWidget>

class Partition;

class QFile;
class QSpinBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QStringList;

class EditMountPointDialogWidget : public QWidget, public Ui::EditMountPointDialogWidgetBase
{
public:
    EditMountPointDialogWidget(QWidget* parent, const Partition& p);
    ~EditMountPointDialogWidget();

    QPushButton& buttonMore() {
        return *m_ButtonMore;
    }
    QLabel& labelName() {
        return *m_LabelNameValue;
    }
    QLineEdit& editPath() {
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
    bool writeMountpoints(const QString& filename);

protected:
    void buttonSelectClicked(bool);
    void buttonMoreClicked(bool);

private:
    void setupOptions(const QStringList& options);
    std::map<QString, QCheckBox*>& boxOptions() {
        return m_BoxOptions;
    }
    const std::map<QString, QCheckBox*>& boxOptions() const {
        return m_BoxOptions;
    }

    const Partition& partition() const {
        return m_Partition;
    }

private:
    FstabEntryList m_fstabEntries;
    FstabEntry *entry;

    const Partition& m_Partition;
    QString m_Options;
    QString m_deviceNode;
    std::map<QString, QCheckBox*> m_BoxOptions;
};

#endif
