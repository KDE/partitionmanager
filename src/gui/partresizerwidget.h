/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
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

#if !defined(PARTRESIZERWIDGET__H)

#define PARTRESIZERWIDGET__H

#include <QWidget>
#include <QLabel>

class Partition;
class PartWidget;
class Device;

class QPaintEvent;
class QResizeEvent;
class QMouseEvent;

/** @brief Widget that allows the user to resize a Partition.
	@author vl@fidra.de
*/
class PartResizerWidget : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(PartResizerWidget)

	public:
		PartResizerWidget(QWidget* parent);

	public:
		void init(Device& d, Partition& p, qint64 sbefore, qint64 safter);

		qint64 sectorsBefore() const { return m_SectorsBefore; } /**< @return sectors free before Partition */
		qint64 sectorsAfter() const { return m_SectorsAfter; } /**< @return sectors free after Partition */
		qint64 totalSectors() const { return m_TotalSectors; } /**< @return total sectors (free + Partition's length) */

		void setMinimumSectors(qint64 s);
		qint64 minimumSectors() const { return m_MinimumSectors; } /**< @return minimum length for Partition */

		void setMaximumSectors(qint64 s);
		qint64 maximumSectors() const { return m_MaximumSectors; } /**< @return maximum length for the Partition */

		void setMoveAllowed(bool b);
		bool moveAllowed() const { return m_MoveAllowed; } /**< @return true if moving the Partition is allowed */

		qint64 maxFirstSector() const { return m_MaxFirstSector; } /**< @return the highest allowed first sector */
		void setMaxFirstSector(qint64 s) { m_MaxFirstSector = s; } /**< @param s the new highest allowed first sector */

		qint64 minLastSector() const { return m_MinLastSector; } /**< @return the lowest allowed last sector */
		void setMinLastSector(qint64 s) { m_MinLastSector = s; } /**< @param s the new lowest allowed last sector */

		bool readOnly() const { return m_ReadOnly; } /**< @return true if the widget is read only */
		void setReadOnly(bool b) { m_ReadOnly = b; } /**< @param b the new value for read only */

		static qint32 handleWidth() { return m_HandleWidth; } /**< @return the handle width in pixels */
		static qint32 handleHeight() { return m_HandleHeight; } /**< @return the handle height in pixels */

	signals:
		void sectorsBeforeChanged(qint64);
		void sectorsAfterChanged(qint64);
		void lengthChanged(qint64);

	public slots:
		bool updateSectors(qint64 newSectorsBefore, qint64 newSectorsAfter);
		bool updateSectorsBefore(qint64 newSectorsBefore, bool enableLengthCheck = true);
		bool updateSectorsAfter(qint64 newSectorsAfter, bool enableLengthCheck = true);
		bool updateLength(qint64 newLength);

	protected:
		Partition& partition() { Q_ASSERT(m_Partition); return *m_Partition; }
		const Partition& partition() const { Q_ASSERT(m_Partition); return *m_Partition; }
		void setPartition(Partition& p) { m_Partition = &p; }

		Device& device() { Q_ASSERT(m_Device); return *m_Device; }
		const Device& device() const { Q_ASSERT(m_Device); return *m_Device; }
		void setDevice(Device& d) { m_Device = &d; }

		void setSectorsBefore(qint64 s) { m_SectorsBefore = s; }
		void setSectorsAfter(qint64 s) { m_SectorsAfter = s; }

		void paintEvent(QPaintEvent* event);
		void resizeEvent(QResizeEvent* event);
		void mousePressEvent(QMouseEvent* event);
		void mouseMoveEvent(QMouseEvent* event);
		void mouseReleaseEvent(QMouseEvent* event);

		PartWidget& partWidget() { Q_ASSERT(m_PartWidget); return *m_PartWidget; }
		const PartWidget& partWidget() const { Q_ASSERT(m_PartWidget); return *m_PartWidget; }

		void updatePositions();

		int partWidgetStart() const;
		int partWidgetWidth() const;

		QLabel& leftHandle() { return m_LeftHandle; }
		QLabel& rightHandle() { return m_RightHandle; }

		qint64 sectorsPerPixel() const;

		void set(qint64 newCap, qint64 newFreeBefore, qint64 newFreeAfter);

		void setTotalSectors(qint64 s) { m_TotalSectors = s; }

		void resizeLogicals();

		bool checkSnap(const Partition& child, qint64 delta) const;

		QWidget* draggedWidget() { return m_DraggedWidget; }
		const QWidget* draggedWidget() const { return m_DraggedWidget; }

	private:
		Device* m_Device;
		Partition* m_Partition;
		PartWidget* m_PartWidget;

		qint64 m_SectorsBefore;
		qint64 m_SectorsAfter;
		qint64 m_TotalSectors;
		qint64 m_MinimumSectors;
		qint64 m_MaximumSectors;
		qint64 m_MaxFirstSector;
		qint64 m_MinLastSector;

		QLabel m_LeftHandle;
		QLabel m_RightHandle;

		QWidget* m_DraggedWidget;
		int m_Hotspot;

		bool m_MoveAllowed;
		bool m_ReadOnly;

		static const qint32 m_HandleWidth;
		static const qint32 m_HandleHeight;
};

#endif

