/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#if !defined(LISTDEVICES__H)

#define LISTDEVICES__H

#include "util/libpartitionmanagerexport.h"

#include "ui_listdevicesbase.h"

#include <QWidget>

#include <kdebug.h>

class Device;
class QPoint;
class PartitionManagerWidget;
class KActionCollection;

/** @brief A list of devices.
	@author vl@fidra.de
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT ListDevices : public QWidget, public Ui::ListDevicesBase
{
	Q_OBJECT
	Q_DISABLE_COPY(ListDevices)

	public:
		ListDevices(QWidget* parent);

	signals:
		void selectionChanged(Device*);

	public:
		void init(KActionCollection* coll, PartitionManagerWidget* pm_widget) { m_ActionCollection = coll; m_PartitionManagerWidget = pm_widget; }

	public slots:
		void updateDevices();

	protected:
		QListWidget& listDevices() { Q_ASSERT(m_ListDevices); return *m_ListDevices; }
		const QListWidget& listDevices() const { Q_ASSERT(m_ListDevices); return *m_ListDevices; }

		PartitionManagerWidget& pmWidget() { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }
		const PartitionManagerWidget& pmWidget() const { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }

		KActionCollection* actionCollection() { return m_ActionCollection; }

	protected slots:
		void on_m_ListDevices_itemSelectionChanged();
		void on_m_ListDevices_customContextMenuRequested(const QPoint& pos);

	private:
		KActionCollection* m_ActionCollection;
		PartitionManagerWidget* m_PartitionManagerWidget;
};

#endif

