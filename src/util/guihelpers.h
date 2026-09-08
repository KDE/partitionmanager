/*
    SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GUIHELPERS_H
#define GUIHELPERS_H

#include <fs/filesystem.h>
#include <util/capacity.h>

#include <vector>

class QComboBox;
class QIcon;
class QPoint;
class QString;
class QTreeWidget;

bool loadBackend();
QIcon createFileSystemColor(FileSystem::Type type, quint32 size);
Capacity::Unit preferredUnit();
void showColumnsContextMenu(const QPoint& p, QTreeWidget& tree);
namespace GuiHelpers
{
FileSystem::Type defaultFileSystem();
std::vector<QColor> fileSystemColorCodesFromSettings();

bool fileSystemSupportsClusterSize(const FileSystem& fs);

void populateClusterSizeCombo(QComboBox& combo, const FileSystem& fs, qint64 fileSystemSizeInBytes, bool keepSelection, qint64 preferredValue = 0);
}

#endif
