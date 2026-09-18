/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef FILESYSTEMPROPERTIESWIDGET_H
#define FILESYSTEMPROPERTIESWIDGET_H

#include <QList>

class FileSystem;
class QGridLayout;
class QWidget;

class FileSystemPropertiesWidget
{
public:
    FileSystemPropertiesWidget(QGridLayout& grid, int row);

    void setFileSystem(const FileSystem& fs);
    void setVisible(bool visible);

    bool isEmpty() const {
        return m_IsEmpty;
    }

private:
    QGridLayout& m_Grid;
    int m_Row;
    QList<QWidget*> m_RowWidgets;
    bool m_IsEmpty;
};

#endif
