/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(PARTTABLEWIDGET_H)

#define PARTTABLEWIDGET_H

#include <gui/partwidgetbase.h>

#include <QLabel>
#include <QStyle>

class PartWidget;
class PartitionTable;

class QResizeEvent;
class QMouseEvent;

/** Widget that represents a PartitionTable.
    @author Volker Lanz <vl@fidra.de>
*/
class PartTableWidget : public PartWidgetBase
{
    Q_OBJECT
    Q_DISABLE_COPY(PartTableWidget)

public:
    explicit PartTableWidget(QWidget* parent);
    qint32 borderWidth() const override {
        return style()->pixelMetric(QStyle::PM_LayoutLeftMargin);    /**< @return border width */
    }
    qint32 borderHeight() const override {
        return style()->pixelMetric(QStyle::PM_LayoutTopMargin);    /**< @return border height */
    }

public:
    void setPartitionTable(const PartitionTable* ptable);

    PartWidget* activeWidget(); /**< @return the active widget or nullptr if none */
    const PartWidget* activeWidget() const; /**< @return the active widget or nullptr if none */

    void setActiveWidget(PartWidget* partWidget);
    void setActivePartition(const Partition* p);
    void clear();
    void setReadOnly(bool b) {
        m_ReadOnly = b;    /**< @param b the new value for read only */
    }
    bool isReadOnly() const {
        return m_ReadOnly;    /** @return true if the widget is read only */
    }

Q_SIGNALS:
    void itemSelectionChanged(PartWidget*);
    void itemDoubleClicked(const PartWidget*);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    const PartitionTable* partitionTable() const {
        return m_PartitionTable;
    }

    QLabel& labelEmpty() {
        return m_LabelEmpty;
    }
    const QLabel& labelEmpty() const {
        return m_LabelEmpty;
    }

private:
    const PartitionTable* m_PartitionTable;
    QLabel m_LabelEmpty;
    bool m_ReadOnly;
};

#endif

