/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/parttablewidget.h"
#include "util/guihelpers.h"
#include "mainwindow.h"

#include <core/partitiontable.h>

#include <gui/partwidget.h>

#include <QMouseEvent>

#include <KLocalizedString>

/** Creates a new PartTableWidget.
    @param parent pointer to the parent widget
*/
PartTableWidget::PartTableWidget(QWidget* parent) :
    PartWidgetBase(parent),
    m_PartitionTable(nullptr),
    m_LabelEmpty(xi18nc("@info", "Please select a device."), this),
    m_ReadOnly(false)
{
    labelEmpty().setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
}

/** Sets the PartitionTable this widget shows.
    @param ptable pointer to the PartitionTable to show. Must not be nullptr.
*/
void PartTableWidget::setPartitionTable(const PartitionTable* ptable)
{
    clear();

    m_PartitionTable = ptable;

    if (partitionTable() != nullptr) {
        for (const auto &p : partitionTable()->children()) {
            PartWidget* w = new PartWidget(this, p);
            w->setVisible(true);
            w->setFileSystemColorCode(GuiHelpers::fileSystemColorCodesFromSettings());
            const auto children = w->childWidgets();
            for (const auto &child : children)
                child->setFileSystemColorCode(GuiHelpers::fileSystemColorCodesFromSettings());
        }
    }

    if (childWidgets().isEmpty()) {
        labelEmpty().setVisible(true);
        labelEmpty().setText(xi18nc("@info", "No valid partition table was found on this device."));
        labelEmpty().resize(size());
    } else {
        labelEmpty().setVisible(false);
        positionChildren(this, partitionTable()->children(), childWidgets());
    }

    update();
}

PartWidget* PartTableWidget::activeWidget()
{
    const auto children = findChildren<PartWidget*>();
    for (auto &pw : children)
        if (pw->isActive())
            return pw;

    return nullptr;
}

const PartWidget* PartTableWidget::activeWidget() const
{
    const auto children = findChildren<PartWidget*>();
    for (const auto &pw : children)
        if (pw->isActive())
            return pw;

    return nullptr;
}

/** Sets a widget active.
    @param p pointer to the PartWidget to set active. May be nullptr.
*/
void PartTableWidget::setActiveWidget(PartWidget* p)
{
    if (isReadOnly() || p == activeWidget())
        return;

    if (activeWidget())
        activeWidget()->setActive(false);

    if (p != nullptr)
        p->setActive(true);

    Q_EMIT itemSelectionChanged(p);

    update();
}

/** Sets a widget for the given Partition active.
    @param p pointer to the Partition whose widget is to be set active. May be nullptr.
*/
void PartTableWidget::setActivePartition(const Partition* p)
{
    if (isReadOnly())
        return;

    const auto children = findChildren<PartWidget*>();
    for (auto &pw : children) {
        if (pw->partition() == p) {
            setActiveWidget(pw);
            return;
        }
    }

    setActiveWidget(nullptr);
}

/** Clears the PartTableWidget.
*/
void PartTableWidget::clear()
{
    setActiveWidget(nullptr);
    m_PartitionTable = nullptr;

    // we might have been invoked indirectly via a widget's context menu, so
    // that its event handler is currently running. therefore, do not delete
    // the part widgets here but schedule them for deletion once the app
    // returns to the main loop (and the event handler has finished).
    for (auto &p : childWidgets()) {
        p->setVisible(false);
        p->deleteLater();
        p->setParent(nullptr);
    }

    update();
}

void PartTableWidget::resizeEvent(QResizeEvent*)
{
    if (partitionTable() == nullptr || childWidgets().isEmpty())
        labelEmpty().resize(size());
    else
        positionChildren(this, partitionTable()->children(), childWidgets());
}

void PartTableWidget::mousePressEvent(QMouseEvent* event)
{
    if (isReadOnly())
        return;

    event->accept();
    PartWidget* child = qobject_cast<PartWidget*>(childAt(event->pos()));
    setActiveWidget(child);
}

void PartTableWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (isReadOnly() || event->button() != Qt::LeftButton)
        return;

    event->accept();

    const PartWidget* child = static_cast<PartWidget*>(childAt(event->pos()));

    if (child != nullptr)
        Q_EMIT itemDoubleClicked(child);
}

#include "moc_parttablewidget.cpp"
