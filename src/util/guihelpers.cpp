/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2015-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "util/guihelpers.h"
#include "config.h"

#include <backend/corebackendmanager.h>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFileInfo>
#include <QIcon>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QRect>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QString>
#include <QTreeWidget>
#include <QVariant>

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
                KMessageBox::error(nullptr,
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
    cc.resize(Config::EnumFileSystem::type::COUNT);
    for (int i = 0; i < Config::EnumFileSystem::type::COUNT; ++i)
    {
        cc[i] = Config::fileSystemColorCode(i);
    }
    return cc;
}

bool fileSystemSupportsClusterSize(const FileSystem& fs)
{
    return fs.supportCreateWithFeatures() != FileSystem::cmdSupportNone
        && fs.availableFeatures().contains(QStringLiteral("cluster-size"));
}

void populateClusterSizeCombo(QComboBox& combo, const FileSystem& fs, qint64 fileSystemSizeInBytes, bool keepSelection, qint64 preferredValue)
{
    qint64 previous = 0;
    if (keepSelection) {
        if (combo.count() > 0)
            previous = combo.currentData().toLongLong();
        else if (preferredValue > 0)
            previous = preferredValue;
        else
            previous = fs.features().value(QStringLiteral("cluster-size")).toLongLong();
    }

    const QSignalBlocker blocker(&combo);
    combo.clear();
    combo.addItem(xi18nc("@item:inlistbox cluster size", "Automatic"), QVariant(qint64(0)));

    const auto sizes = fs.supportedClusterSizes(fileSystemSizeInBytes);
    for (const qint64 bytes : sizes)
        combo.addItem(Capacity::formatByteSize(bytes), QVariant(bytes));

    if (previous > 0 && !sizes.contains(previous)) {
        combo.addItem(xi18nc("@item:inlistbox a cluster size that is no longer valid",
                             "%1 (not valid for this size)", Capacity::formatByteSize(previous)),
                      QVariant(previous));
    }

    int index = 0;
    if (previous > 0) {
        const int found = combo.findData(QVariant(previous));
        if (found != -1)
            index = found;
    }
    combo.setCurrentIndex(index);
}

}
