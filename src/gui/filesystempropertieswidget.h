/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef FILESYSTEMPROPERTIESWIDGET_H
#define FILESYSTEMPROPERTIESWIDGET_H

#include <QWidget>

class FileSystem;
class QVBoxLayout;

class FileSystemPropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileSystemPropertiesWidget(QWidget* parent = nullptr);

    void setFileSystem(const FileSystem& fs);

    bool isEmpty() const {
        return m_IsEmpty;
    }

private:
    QVBoxLayout* m_Layout;
    QWidget* m_Content;
    bool m_IsEmpty;
};

#endif
