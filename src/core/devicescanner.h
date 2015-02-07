/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
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

#ifndef DEVICESCANNER_H
#define DEVICESCANNER_H

#include <QThread>

class OperationStack;

/** Thread to scan for all available Devices on this computer.

	This class is used to find all Devices on the computer and to create new Device instances for each of them. It's subclassing QThread to run asynchronously.

	@author Volker Lanz <vl@fidra.de>
*/
class DeviceScanner : public QThread
{
	Q_OBJECT

	public:
		DeviceScanner(QObject* parent, OperationStack& ostack);

	public:
		void clear(); /**< clear Devices and the OperationStack */
		void scan(); /**< do the actual scanning; blocks if called directly */
		void setupConnections();

	Q_SIGNALS:
		void progress(const QString& device_node, int progress);

	protected:
		virtual void run();
		OperationStack& operationStack() { return m_OperationStack; }
		const OperationStack& operationStack() const { return m_OperationStack; }

	private:
		OperationStack& m_OperationStack;
};

#endif
