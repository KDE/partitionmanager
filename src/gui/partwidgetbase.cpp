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

#include "gui/partwidgetbase.h"
#include "gui/partwidget.h"

#include "core/partition.h"

#include <kdebug.h>
#include <cmath>

const qint32 PartWidgetBase::m_Spacing = 3;
const qint32 PartWidgetBase::m_BorderWidth = 5;
const qint32 PartWidgetBase::m_BorderHeight = 5;
const qint32 PartWidgetBase::m_MinWidth = 20;

template<typename T>
T sum(const QList<T>& list)
{
	T rval = 0;
	foreach(const T& val, list)
		rval += val;
	return rval;
}

bool distributeLostPixels(QList<qint32>& childrenWidth, qint32 lostPixels)
{
	if (lostPixels == 0 || childrenWidth.size() == 0)
		return false;

	while (lostPixels > 0)
		for (qint32 i = 0; i < childrenWidth.size() && lostPixels > 0; i++)
		{
			childrenWidth[i]++;
			lostPixels--;
		}

	return true;
}

bool levelChildrenWidths(QList<qint32>& childrenWidth, const qint32 destWidgetWidth)
{
	if (childrenWidth.size() == 0)
		return false;

	distributeLostPixels(childrenWidth, destWidgetWidth - sum(childrenWidth));
	
	// if we find out a partition is too narrow, adjust its screen
	// width to minWidth() and increase adjust by how much we had to increase the
	// screen width. thus, in the end, we have the number of pixels we need
	// to find somewhere else in adjust.
	qint32 adjust = 0;
	for (qint32 i = 0; i < childrenWidth.size(); i++)
		if (childrenWidth[i] < PartWidgetBase::minWidth())
		{
			adjust += PartWidgetBase::minWidth() - childrenWidth[i];
			childrenWidth[i] = PartWidgetBase::minWidth();
		}

	// find out how many partitions are wide enough to have their width reduced; we'd love to
	// check for w > minWidth - (pixels_to_reduce_by), but that last value _depends_ on the
	// number we're trying to find here...
	qint32 numReducable = 0;
	foreach(int w, childrenWidth)
		if (w > PartWidgetBase::minWidth())
			numReducable++;
	
	// no need to do anything... or nothing can be done because all are too narrow
	if (adjust == 0 || numReducable == 0)
		return false;

	// if we have adjusted one or more partitions (and not ALL of them, because in that
	// case, nothing will help us), go through the partitions again and reduce the
	// on screen widths of those big enough anyway
	const qint32 reduce = ceil(1.0 * adjust / numReducable);
	for (qint32 i = 0; i < childrenWidth.size(); i++)
		if (childrenWidth[i] > PartWidgetBase::minWidth())
			childrenWidth[i] -= reduce;

	// distribute pixels lost due to rounding errors
	distributeLostPixels(childrenWidth, destWidgetWidth - sum(childrenWidth));
	
	return true;
}

void PartWidgetBase::positionChildren(const QWidget* destWidget, const PartitionNode::Partitions& partitions, QList<PartWidget*>& widgets)
{
	if (partitions.size() == 0)
		return;
	
	QList<qint32> childrenWidth;
	const qint32 destWidgetWidth = destWidget->width() - 2 * borderWidth() - (partitions.size() - 1) * spacing();

	if (destWidgetWidth < 0)
		return;
	
	qint64 totalLength = 0;
	foreach (const Partition* p, partitions)
		totalLength += p->length();

	// calculate unleveled width for each child and store it
	for (int i = 0; i < partitions.size(); i++)
		childrenWidth.append(partitions[i]->length() * destWidgetWidth / totalLength);

	// now go level the widths as long as required
 	while (levelChildrenWidths(childrenWidth, destWidgetWidth))
		;

	// move the children to their positions and resize them
	for (int i = 0, x = borderWidth(); i < widgets.size(); i++)
	{
		widgets[i]->setMinimumWidth(minWidth());
		widgets[i]->move(x, borderHeight());
		widgets[i]->resize(childrenWidth[i], destWidget->height() - 2 * borderHeight());
		x += childrenWidth[i] + spacing();
	}
}
