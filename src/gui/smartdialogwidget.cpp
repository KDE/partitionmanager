/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/smartdialogwidget.h"

#include "util/guihelpers.h"

#include <KLocalizedString>

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QPoint>
#include <QTextDocument>

#include <config.h>

class SmartAttrDelegate : public QStyledItemDelegate
{
public:
    SmartAttrDelegate() : QStyledItemDelegate() {}

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

void SmartAttrDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString text = index.data().toString();

    painter->save();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter);

    QTextDocument doc;
    doc.setHtml(text);
    doc.setPageSize(option.rect.size());

    painter->setClipRect(option.rect);
    qint32 offset = (option.rect.height() - doc.size().height()) / 2;
    if (offset < 0)
        offset = 0;

    painter->translate(option.rect.x(), option.rect.y() + offset);
    doc.drawContents(painter);

    painter->restore();
}

SmartDialogWidget::SmartDialogWidget(QWidget* parent) :
    QWidget(parent),
    m_SmartAttrDelegate(new SmartAttrDelegate())
{
    setupUi(this);
    setupConnections();

    loadConfig();

    treeSmartAttributes().setItemDelegateForColumn(1, m_SmartAttrDelegate);
    treeSmartAttributes().header()->setContextMenuPolicy(Qt::CustomContextMenu);
}

SmartDialogWidget::~SmartDialogWidget()
{
    saveConfig();
    delete m_SmartAttrDelegate;
}

void SmartDialogWidget::loadConfig()
{
    QList<int> colWidths = Config::treeSmartAttributesColumnWidths();
    QList<int> colPositions = Config::treeSmartAttributesColumnPositions();
    QList<int> colVisible = Config::treeSmartAttributesColumnVisible();
    QHeaderView* header = treeSmartAttributes().header();

    for (int i = 0; i < treeSmartAttributes().columnCount(); i++) {
        if (colPositions[0] != -1 && colPositions.size() > i)
            header->moveSection(header->visualIndex(i), colPositions[i]);

        if (colVisible[0] != -1 && colVisible.size() > i)
            treeSmartAttributes().setColumnHidden(i, colVisible[i] == 0);

        if (colWidths[0] != -1 && colWidths.size() > i)
            treeSmartAttributes().setColumnWidth(i, colWidths[i]);
    }
}

void SmartDialogWidget::saveConfig() const
{
    QList<int> colWidths;
    QList<int> colPositions;
    QList<int> colVisible;

    for (int i = 0; i < treeSmartAttributes().columnCount(); i++) {
        colPositions.append(treeSmartAttributes().header()->visualIndex(i));
        colVisible.append(treeSmartAttributes().isColumnHidden(i) ? 0 : 1);
        colWidths.append(treeSmartAttributes().columnWidth(i));
    }

    Config::setTreeSmartAttributesColumnPositions(colPositions);
    Config::setTreeSmartAttributesColumnVisible(colVisible);
    Config::setTreeSmartAttributesColumnWidths(colWidths);

    Config::self()->save();
}

void SmartDialogWidget::setupConnections()
{
    connect(treeSmartAttributes().header(), &QHeaderView::customContextMenuRequested, this, &SmartDialogWidget::onHeaderContextMenu);
}

void SmartDialogWidget::onHeaderContextMenu(const QPoint& p)
{
    showColumnsContextMenu(p, treeSmartAttributes());
}
