/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(DEVICEPROPSDIALOG_H)

#define DEVICEPROPSDIALOG_H

#include <QDialog>

class Device;
class DevicePropsWidget;

class QDialogButtonBox;
class QPushButton;
class QVBoxLayout;
class QWidget;
class QString;

/** Show Device properties.

    Dialog that shows a Device's properties.

    @author Volker Lanz <vl@fidra.de>
*/
class DevicePropsDialog : public QDialog
{
    Q_DISABLE_COPY(DevicePropsDialog)

public:
    DevicePropsDialog(QWidget* parent, Device& d);
    ~DevicePropsDialog();

public:
    bool cylinderBasedAlignment() const;
    bool sectorBasedAlignment() const;

protected:
    void setupDialog();
    void setupConnections();

    Device& device() {
        return m_Device;
    }
    const Device& device() const {
        return m_Device;
    }

    DevicePropsWidget& dialogWidget() {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }
    const DevicePropsWidget& dialogWidget() const {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }

    void setDirty(bool);
    void onButtonSmartMore(bool);

private:
    Device& m_Device;
    DevicePropsWidget* m_DialogWidget;

    QDialogButtonBox* dialogButtonBox;
    QPushButton* okButton;
    QPushButton* cancelButton;
    QVBoxLayout *mainLayout;
};

#endif
