/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(SMARTDIALOGWIDGET_H)

#define SMARTDIALOGWIDGET_H

#include "ui_smartdialogwidgetbase.h"

class QStyledItemDelegate;
class QPoint;

/** Central widget in the SmartDialogWidget
    @author Volker Lanz <vl@fidra.de>
*/
class SmartDialogWidget : public QWidget, public Ui::SmartDialogWidgetBase
{
public:
    explicit SmartDialogWidget(QWidget* parent);
    ~SmartDialogWidget();

public:
    QLabel& statusText() {
        Q_ASSERT(m_LabelSmartStatusText);
        return *m_LabelSmartStatusText;
    }
    QLabel& statusIcon() {
        Q_ASSERT(m_LabelSmartStatusIcon);
        return *m_LabelSmartStatusIcon;
    }
    QLabel& modelName() {
        Q_ASSERT(m_LabelSmartModelName);
        return *m_LabelSmartModelName;
    }
    QLabel& firmware() {
        Q_ASSERT(m_LabelSmartFirmware);
        return *m_LabelSmartFirmware;
    }
    QLabel& serialNumber() {
        Q_ASSERT(m_LabelSmartSerialNumber);
        return *m_LabelSmartSerialNumber;
    }
    QLabel& temperature() {
        Q_ASSERT(m_LabelSmartTemperature);
        return *m_LabelSmartTemperature;
    }
    QLabel& badSectors() {
        Q_ASSERT(m_LabelSmartBadSectors);
        return *m_LabelSmartBadSectors;
    }
    QLabel& poweredOn() {
        Q_ASSERT(m_LabelSmartPoweredOn);
        return *m_LabelSmartPoweredOn;
    }
    QLabel& powerCycles() {
        Q_ASSERT(m_LabelSmartPowerCycles);
        return *m_LabelSmartPowerCycles;
    }

    QLabel& selfTests() {
        Q_ASSERT(m_LabelSmartSelfTests);
        return *m_LabelSmartSelfTests;
    }
    QLabel& overallAssessment() {
        Q_ASSERT(m_LabelSmartOverallAssessment);
        return *m_LabelSmartOverallAssessment;
    }

    QTreeWidget& treeSmartAttributes() {
        Q_ASSERT(m_TreeSmartAttributes);
        return *m_TreeSmartAttributes;
    }
    const QTreeWidget& treeSmartAttributes() const {
        Q_ASSERT(m_TreeSmartAttributes);
        return *m_TreeSmartAttributes;
    }

protected:
    void setupConnections();
    void loadConfig();
    void saveConfig() const;
    void onHeaderContextMenu(const QPoint& p);

private:
    QStyledItemDelegate* m_SmartAttrDelegate;
};

#endif
