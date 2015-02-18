/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
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

#if !defined(HELPERS__H)

#define HELPERS__H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

class KAboutData;
class QString;
class QIcon;
class QPoint;
class QTreeWidget;


LIBKPMCORE_EXPORT void registerMetaTypes();
LIBKPMCORE_EXPORT bool checkPermissions();

LIBKPMCORE_EXPORT KAboutData* createPartitionManagerAboutData();

LIBKPMCORE_EXPORT bool caseInsensitiveLessThan(const QString& s1, const QString& s2);
LIBKPMCORE_EXPORT bool naturalLessThan(const QString& s1, const QString& s2);

LIBKPMCORE_EXPORT QIcon createFileSystemColor(FileSystem::Type type, quint32 size);

LIBKPMCORE_EXPORT void showColumnsContextMenu(const QPoint& p, QTreeWidget& tree);

LIBKPMCORE_EXPORT bool loadBackend();

LIBKPMCORE_EXPORT bool checkAccessibleDevices();

#endif
