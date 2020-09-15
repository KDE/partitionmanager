/*
    SPDX-FileCopyrightText: 2010-2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(INFOPANE_H)

#define INFOPANE_H

#include <QWidget>

class Partition;
class Device;

class QGridLayout;
class QString;

/** Show information about Partitions and Devices

    Child widget of the QDockWidget to show some details about the currently selected Partition
    or Device

    @author Volker Lanz <vl@fidra.de>
*/
class InfoPane : public QWidget
{
    Q_DISABLE_COPY(InfoPane)

public:
    explicit InfoPane(QWidget* parent = nullptr);

public:
    void showPartition(Qt::DockWidgetArea area, const Partition& p);
    void showDevice(Qt::DockWidgetArea area, const Device& d);
    void clear();

protected:
    void createLabels(const QString& title, const QString& value, const int cols, int& x, int& y);
    int createHeader(const QString& title, const int cols);
    QGridLayout& gridLayout() {
        Q_ASSERT(m_GridLayout);
        return *m_GridLayout;
    }
    qint32 cols(Qt::DockWidgetArea area) const;

private:
    QGridLayout* m_GridLayout;
};

#endif
