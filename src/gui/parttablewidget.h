/*************************************************************************
 *  Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(PARTTABLEWIDGET_H)

#define PARTTABLEWIDGET_H

#include <gui/partwidgetbase.h>

#include <QLabel>

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
        return 0;    /**< @return border width */
    }
    qint32 borderHeight() const override {
        return 0;    /**< @return border height */
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

