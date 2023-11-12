/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2018 Abhijeet Sharma <sharma.abhijeet2096@gmail.com>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(PARTPROPSWIDGET_H)

#define PARTPROPSWIDGET_H

#include "ui_partpropswidgetbase.h"

#include "mainwindow.h"
#include "util/guihelpers.h"

/** Central widget in the PartPropsDialog.
    @author Volker Lanz <vl@fidra.de>
*/
class PartPropsWidget : public QWidget, public Ui::PartPropsWidgetBase
{
public:
    explicit PartPropsWidget(QWidget* parent) : QWidget(parent) {
        setupUi(this);

        m_PartWidget->setFileSystemColorCode(GuiHelpers::fileSystemColorCodesFromSettings());
        MainWindow* mw = nullptr;
        const auto widgets = qApp->topLevelWidgets();
        for (auto &widget : widgets)
        {
            mw = qobject_cast< MainWindow* >( widget );
            if ( mw )
                break;
        }
        if ( mw )
        {
            connect( mw, &MainWindow::settingsChanged,
                     this, [this]
            {
                m_PartWidget->setFileSystemColorCode(GuiHelpers::fileSystemColorCodesFromSettings());
            });
        }

    }

public:
    PartWidget& partWidget() {
        Q_ASSERT(m_PartWidget);
        return *m_PartWidget;
    }

    QLabel& mountPoint() {
        Q_ASSERT(m_LabelMountPoint);
        return *m_LabelMountPoint;
    }
    QLabel& devicePath() {
        Q_ASSERT(m_LabelDevicePath);
        return *m_LabelDevicePath;
    }
    QLabel& role() {
        Q_ASSERT(m_LabelRole);
        return *m_LabelRole;
    }
    QLabel& capacity() {
        Q_ASSERT(m_LabelCapacity);
        return *m_LabelCapacity;
    }

    QLabel& textAvailable() {
        Q_ASSERT(m_LabelTextAvailable);
        return *m_LabelTextAvailable;
    }
    QLabel& available() {
        Q_ASSERT(m_LabelAvailable);
        return *m_LabelAvailable;
    }

    QLabel& textUsed() {
        Q_ASSERT(m_LabelTextUsed);
        return *m_LabelTextUsed;
    }
    QLabel& used() {
        Q_ASSERT(m_LabelUsed);
        return *m_LabelUsed;
    }

    QLabel& textFileSystem() {
        Q_ASSERT(m_LabelFileSystem);
        return *m_LabelFileSystem;
    }
    QComboBox& fileSystem() {
        Q_ASSERT(m_ComboFileSystem);
        return *m_ComboFileSystem;
    }
    const QComboBox& fileSystem() const {
        Q_ASSERT(m_ComboFileSystem);
        return *m_ComboFileSystem;
    }

    QCheckBox& checkRecreate() {
        Q_ASSERT(m_CheckRecreate);
        return *m_CheckRecreate;
    }

    QLabel& firstSector() {
        Q_ASSERT(m_LabelFirstSector);
        return *m_LabelFirstSector;
    }
    QLabel& lastSector() {
        Q_ASSERT(m_LabelLastSector);
        return *m_LabelLastSector;
    }
    QLabel& numSectors() {
        Q_ASSERT(m_LabelNumSectors);
        return *m_LabelNumSectors;
    }
    QLabel& status() {
        Q_ASSERT(m_LabelStatus);
        return *m_LabelStatus;
    }

    QLabel& textUuid() {
        Q_ASSERT(m_LabelTextUuid);
        return *m_LabelTextUuid;
    }
    QLabel& uuid() {
        Q_ASSERT(m_LabelUuid);
        return *m_LabelUuid;
    }

    QLabel& textLabel() {
        Q_ASSERT(m_LabelTextLabel);
        return *m_LabelTextLabel;
    }
    QLineEdit& label() {
        Q_ASSERT(m_EditLabel);
        return *m_EditLabel;
    }

    QLabel& partitionTextUuid() {
        Q_ASSERT(m_LabelTextPartitionUuid);
        return *m_LabelTextPartitionUuid;
    }
    QLabel& partitionUuid() {
        Q_ASSERT(m_LabelPartitionUuid);
        return *m_LabelPartitionUuid;
    }

    QLabel& partitionTextLabel() {
        Q_ASSERT(m_LabelTextPartitionLabel);
        return *m_LabelTextPartitionLabel;
    }
    QLabel& partitionLabel() {
        Q_ASSERT(m_LabelPartitionLabel);
        return *m_LabelPartitionLabel;
    }
    const QLineEdit& label() const {
        Q_ASSERT(m_EditLabel);
        return *m_EditLabel;
    }
    QLabel& noSetLabel() {
        Q_ASSERT(m_LabelTextNoSetLabel);
        return *m_LabelTextNoSetLabel;
    }

    QLabel& textFlags() {
        Q_ASSERT(m_LabelTextFlags);
        return *m_LabelTextFlags;
    }
    QListWidget& listFlags() {
        Q_ASSERT(m_ListFlags);
        return *m_ListFlags;
    }
    const QListWidget& listFlags() const {
        Q_ASSERT(m_ListFlags);
        return *m_ListFlags;
    }
    QFrame& lineFlags() {
        Q_ASSERT(m_LineListFlags);
        return *m_LineListFlags;
    }

    void showAvailable(bool b) {
        available().setVisible(b);
        textAvailable().setVisible(b);
    }
    void showUsed(bool b) {
        used().setVisible(b);
        textUsed().setVisible(b);
    }
    void showFileSystem(bool b) {
        fileSystem().setVisible(b);
        textFileSystem().setVisible(b);
    }
    void showCheckRecreate(bool b) {
        checkRecreate().setVisible(b);
    }
    void showListFlags(bool b) {
        listFlags().setVisible(b);
        textFlags().setVisible(b);
        lineFlags().setVisible(b);
    }
    void showLabel(bool b) {
        textLabel().setVisible(b);
        label().setVisible(b);
    }
    void showUuid(bool b) {
        textUuid().setVisible(b);
        uuid().setVisible(b);
    }
};

#endif
