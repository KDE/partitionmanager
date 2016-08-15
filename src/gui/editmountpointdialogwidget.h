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

#if !defined(EDITMOUNTPOINTDIALOGWIDGET__H)

#define EDITMOUNTPOINTDIALOGWIDGET__H

#include "ui_editmountpointdialogwidgetbase.h"

#include <QMap>
#include <QString>

class Partition;

class QWidget;
class QSpinBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QStringList;
class QFile;

class MountEntry;

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

    bool acceptChanges();
    bool writeMountpoints(const QString& filename);

protected:
    void buttonSelectClicked(bool);
    void buttonMoreClicked(bool);

private:
    void setupOptions(const QStringList& options);
    QMap<QString, QCheckBox*>& boxOptions() {
        return m_BoxOptions;
    }
    const QMap<QString, QCheckBox*>& boxOptions() const {
        return m_BoxOptions;
    }
    bool readMountpoints(const QString& filename);
    QMap<QString, MountEntry*>& mountPoints() {
        return m_MountPoints;
    }
    const Partition& partition() const {
        return m_Partition;
    }

private:
    const Partition& m_Partition;
    QMap<QString, MountEntry*> m_MountPoints;
    QString m_Options;
    QString m_deviceNode;
    QMap<QString, QCheckBox*> m_BoxOptions;
};

#endif
