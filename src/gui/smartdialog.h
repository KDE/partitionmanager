/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(SMARTDIALOG_H)

#define SMARTDIALOG_H

#include <QDialog>

class Device;
class SmartDialogWidget;

class QWidget;
class QString;
class QPoint;
class QDialogButtonBox;

/** Show SMART properties.

    Dialog that shows SMART status and properties for a device

    @author Volker Lanz <vl@fidra.de>
*/
class SmartDialog : public QDialog
{
    Q_DISABLE_COPY(SmartDialog)

public:
    SmartDialog(QWidget* parent, Device& d);
    ~SmartDialog();

protected:
    void saveSmartReport();

    void setupDialog();
    void setupConnections();

    Device& device() {
        return m_Device;
    }
    const Device& device() const {
        return m_Device;
    }

    SmartDialogWidget& dialogWidget() {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }
    const SmartDialogWidget& dialogWidget() const {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }

    QString toHtml() const;

private:
    Device& m_Device;
    SmartDialogWidget* m_DialogWidget;
    QDialogButtonBox* buttonBox;
};

#endif
