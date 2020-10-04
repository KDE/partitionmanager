/*
    SPDX-FileCopyrightText: 2009-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/treelog.h"
#include "gui/partitionmanagerwidget.h"
#include "util/guihelpers.h"
#include "config.h"

#include <util/globallog.h>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <KIO/CopyJob>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>

/** Creates a new TreeLog instance.
    @param parent the parent widget
*/
TreeLog::TreeLog(QWidget* parent) :
    QWidget(parent),
    Ui::TreeLogBase()
{
    setupUi(this);

    treeLog().header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeLog().header(), &QHeaderView::customContextMenuRequested, this, &TreeLog::onHeaderContextMenu);

}

TreeLog::~TreeLog()
{
    saveConfig();
}

void TreeLog::init()
{
    loadConfig();
}

void TreeLog::loadConfig()
{
    QList<int> colWidths = Config::treeLogColumnWidths();
    QList<int> colPositions = Config::treeLogColumnPositions();
    QList<int> colVisible = Config::treeLogColumnVisible();
    QHeaderView* header = treeLog().header();

    for (int i = 0; i < treeLog().columnCount(); i++) {
        if (colPositions[0] != -1 && colPositions.size() > i)
            header->moveSection(header->visualIndex(i), colPositions[i]);

        if (colVisible[0] != -1 && colVisible.size() > i)
            treeLog().setColumnHidden(i, colVisible[i] == 0);

        if (colWidths[0] != -1 && colWidths.size() > i)
            treeLog().setColumnWidth(i, colWidths[i]);
    }
}

void TreeLog::saveConfig() const
{
    QList<int> colWidths;
    QList<int> colPositions;
    QList<int> colVisible;

    for (int i = 0; i < treeLog().columnCount(); i++) {
        colPositions.append(treeLog().header()->visualIndex(i));
        colVisible.append(treeLog().isColumnHidden(i) ? 0 : 1);
        colWidths.append(treeLog().columnWidth(i));
    }

    Config::setTreeLogColumnPositions(colPositions);
    Config::setTreeLogColumnVisible(colVisible);
    Config::setTreeLogColumnWidths(colWidths);

    Config::self()->save();
}

void TreeLog::onHeaderContextMenu(const QPoint& pos)
{
    showColumnsContextMenu(pos, treeLog());
}

void TreeLog::onClearLog()
{
    while (QTreeWidgetItem* item = treeLog().takeTopLevelItem(0))
        delete item;
}

void TreeLog::onSaveLog()
{
    const QUrl url = QFileDialog::getSaveFileUrl();

    if (!url.isEmpty()) {
        QTemporaryFile tempFile;

        if (!tempFile.open()) {
            KMessageBox::error(this, xi18nc("@info", "Could not create temporary output file to save <filename>%1</filename>.", url.fileName()), xi18nc("@title:window", "Error Saving Log File"));
            return;
        }

        QTextStream stream(&tempFile);

        for (qint32 idx = 0; idx < treeLog().topLevelItemCount(); idx++) {
            QTreeWidgetItem* item = treeLog().topLevelItem(idx);
            stream << item->text(1) << ": " << item->text(2) << "\n";
        }

        tempFile.close();

        KIO::CopyJob* job = KIO::move(QUrl::fromLocalFile(tempFile.fileName()), url, KIO::HideProgressInfo);
        job->exec();
        if (job->error())
            job->uiDelegate()->showErrorMessage();
    }
}

void TreeLog::onNewLogMessage(Log::Level logLevel, const QString& s)
{
    static const QString icons[] = {
        QStringLiteral("tools-report-bug"),
        QStringLiteral("dialog-information"),
        QStringLiteral("dialog-warning"),
        QStringLiteral("dialog-error")
    };

    qDebug() << s;

    if (static_cast<int>(logLevel) >= Config::minLogLevel()) {
        QTreeWidgetItem* item = new QTreeWidgetItem();

        item->setIcon(0, QIcon::fromTheme(icons[static_cast<int>(logLevel)]));
        item->setText(1, QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")));
        item->setText(2, s);

        treeLog().addTopLevelItem(item);
        treeLog().scrollToBottom();
    }
}
