/*************************************************************************
 *  Copyright (C) 2008-2010 by Volker Lanz <vl@fidra.de>                 *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "util/guihelpers.h"
#include "config.h"

#include <backend/corebackendmanager.h>

#include <QAction>
#include <QApplication>
#include <QFileInfo>
#include <QIcon>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QRect>
#include <QStandardPaths>
#include <QString>
#include <QTreeWidget>

#include <KLocalizedString>
#include <KMessageBox>

#include <unistd.h>

QIcon createFileSystemColor(FileSystem::Type type, quint32 size)
{
    QPixmap pixmap(size, size);
    QPainter painter(&pixmap);
    painter.setPen(QColor(0, 0, 0));
    painter.setBrush(Config::fileSystemColorCode(static_cast<int>(type)));
    painter.drawRect(QRect(0, 0, pixmap.width() - 1, pixmap.height() - 1));
    painter.end();

    return QIcon(pixmap);
}

bool loadBackend()
{
    if (CoreBackendManager::self()->load(Config::backend()) == false) {
        if (CoreBackendManager::self()->load(CoreBackendManager::defaultBackendName())) {
            if (!Config::firstRun())
                KMessageBox::sorry(nullptr,
                               xi18nc("@info", "<para>The configured backend plugin \"%1\" could not be loaded.</para>"
                                      "<para>Loading the default backend plugin \"%2\" instead.</para>",
                                      Config::backend(), CoreBackendManager::defaultBackendName()),
                               xi18nc("@title:window", "Error: Could Not Load Backend Plugin"));
            Config::setBackend(CoreBackendManager::defaultBackendName());
        } else {
            KMessageBox::error(nullptr,
                               xi18nc("@info", "<para>Neither the configured (\"%1\") nor the default (\"%2\") backend "
                                      "plugin could be loaded.</para><para>Please check your installation.</para>",
                                      Config::backend(), CoreBackendManager::defaultBackendName()),
                               xi18nc("@title:window", "Error: Could Not Load Backend Plugin"));
            return false;
        }
    }

    return true;
}

Capacity::Unit preferredUnit()
{
    return static_cast<Capacity::Unit>(Config::preferredUnit());
}

void showColumnsContextMenu(const QPoint& p, QTreeWidget& tree)
{
    QMenu headerMenu(xi18nc("@title:menu", "Columns"));

    QHeaderView* header = tree.header();

    for (qint32 i = 0; i < tree.model()->columnCount(); i++) {
        const int idx = header->logicalIndex(i);
        const QString text = tree.model()->headerData(idx, Qt::Horizontal).toString();

        QAction* action = headerMenu.addAction(text);
        action->setCheckable(true);
        action->setChecked(!header->isSectionHidden(idx));
        action->setData(idx);
        action->setEnabled(idx > 0);
    }

    QAction* action = headerMenu.exec(tree.header()->mapToGlobal(p));

    if (action != nullptr) {
        const bool hidden = !action->isChecked();
        tree.setColumnHidden(action->data().toInt(), hidden);
        if (!hidden)
            tree.resizeColumnToContents(action->data().toInt());
    }
}

namespace GuiHelpers
{

FileSystem::Type defaultFileSystem()
{
    return static_cast<FileSystem::Type>(Config::defaultFileSystem());
}

std::vector<QColor> fileSystemColorCodesFromSettings()
{
    std::vector<QColor> cc;
    cc.reserve(Config::EnumFileSystem::type::COUNT);
    for (int i = 0; i < Config::EnumFileSystem::type::COUNT; ++i)
    {
        cc[i] = Config::fileSystemColorCode(i);
    }
    return cc;
}

}
