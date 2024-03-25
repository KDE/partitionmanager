/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(SIZEDIALOGWIDGET_H)

#define SIZEDIALOGWIDGET_H

#include "ui_sizedialogwidgetbase.h"

#include <QWidget>

/** Central widget for the SizeDialogBase
    @author Volker Lanz <vl@fidra.de>
*/
class SizeDialogWidget : public QWidget, public Ui::SizeDialogWidgetBase
{
public:
    SizeDialogWidget(QWidget* parent) : QWidget(parent), Ui::SizeDialogWidgetBase() {
        setupUi(this);
        hidePosixPermissions();
    }

public:
    PartResizerWidget& partResizerWidget() {
        Q_ASSERT(m_PartResizerWidget);
        return *m_PartResizerWidget;
    }

    QLabel& labelFreeBefore() {
        Q_ASSERT(m_LabelFreeBefore);
        return *m_LabelFreeBefore;
    }
    QDoubleSpinBox& spinFreeBefore() {
        Q_ASSERT(m_SpinFreeBefore);
        return *m_SpinFreeBefore;
    }
    QLabel& labelFreeAfter() {
        Q_ASSERT(m_LabelFreeAfter);
        return *m_LabelFreeAfter;
    }
    QDoubleSpinBox& spinFreeAfter() {
        Q_ASSERT(m_SpinFreeAfter);
        return *m_SpinFreeAfter;
    }
    QDoubleSpinBox& spinCapacity() {
        Q_ASSERT(m_SpinCapacity);
        return *m_SpinCapacity;
    }

    QLabel& labelMinSize() {
        Q_ASSERT(m_LabelMinSize);
        return *m_LabelMinSize;
    }
    QLabel& labelMaxSize() {
        Q_ASSERT(m_LabelMaxSize);
        return *m_LabelMaxSize;
    }

    QRadioButton& radioPrimary() {
        Q_ASSERT(m_RadioPrimary);
        return *m_RadioPrimary;
    }
    QRadioButton& radioExtended() {
        Q_ASSERT(m_RadioExtended);
        return *m_RadioExtended;
    }
    QRadioButton& radioLogical() {
        Q_ASSERT(m_RadioLogical);
        return *m_RadioLogical;
    }

    QRadioButton& radioRootPermissions() {
        Q_ASSERT(m_permissionOnlyRoot);
        return *m_permissionOnlyRoot;
    }

    QComboBox& comboFileSystem() {
        Q_ASSERT(m_ComboFileSystem);
        return *m_ComboFileSystem;
    }

    QCheckBox& checkBoxEncrypt() {
        Q_ASSERT(m_CheckBoxEncrypt);
        return *m_CheckBoxEncrypt;
    }

    QCheckBox& checkBoxLuks2() {
        Q_ASSERT(m_CheckBoxLuks2);
        return *m_CheckBoxLuks2;
    }

    KNewPasswordWidget& editPassphrase() {
        Q_ASSERT(m_EditPassphrase);
        return *m_EditPassphrase;
    }
    QLabel& textLVName() {
        Q_ASSERT(m_LabelTextLVName);
        return *m_LabelTextLVName;
    }
    QLineEdit& lvName() {
        Q_ASSERT(m_EditLVName);
        return *m_EditLVName;
    }

    QLabel& textLabel() {
        Q_ASSERT(m_LabelTextLabel);
        return *m_LabelTextLabel;
    }
    QLineEdit& label() {
        Q_ASSERT(m_EditLabel);
        return *m_EditLabel;
    }
    const QLineEdit& label() const {
        Q_ASSERT(m_EditLabel);
        return *m_EditLabel;
    }
    QLabel& noSetLabel() {
        Q_ASSERT(m_LabelTextNoSetLabel);
        return *m_LabelTextNoSetLabel;
    }

    void hideRole() {
        delete m_LabelRole;
        m_LabelRole = nullptr;
        delete m_RadioPrimary;
        m_RadioPrimary = nullptr;
        delete m_RadioExtended;
        m_RadioExtended = nullptr;
        delete m_RadioLogical;
        m_RadioLogical = nullptr;
        delete m_groupBox_type;
        m_groupBox_type = nullptr;
    }
    void hideFileSystem() {
        delete m_LabelFileSystem;
        m_LabelFileSystem = nullptr;
        delete m_ComboFileSystem;
        m_ComboFileSystem = nullptr;
        delete m_CheckBoxEncrypt;
        m_CheckBoxEncrypt = nullptr;
        delete m_EditPassphrase;
        m_EditPassphrase = nullptr;
    }
    void hideLabel() {
        delete m_LabelTextLabel;
        m_LabelTextLabel = nullptr;
        delete m_EditLabel;
        m_EditLabel = nullptr;
        delete m_LabelTextNoSetLabel;
        m_LabelTextNoSetLabel = nullptr;
    }

    void hidePosixPermissions() {
        m_groupBox_permissions->hide();
        m_labelPermission->hide();
    }

    void showPosixPermissions() {
        m_groupBox_permissions->show();
        m_labelPermission->show();
    }

    bool isPermissionOnlyRoot() const {
        return !m_permissionEveryone->isChecked();
    }

    void hideBeforeAndAfter() {
        labelFreeBefore().hide();
        spinFreeBefore().hide();
        labelFreeAfter().hide();
        spinFreeAfter().hide();
    }
};

#endif
