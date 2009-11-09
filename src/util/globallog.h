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

#if !defined(GLOBALLOG__H)

#define GLOBALLOG__H

#include "util/libpartitionmanagerexport.h"

#include <QString>
#include <QObject>
#include <qglobal.h>

class log
{
	public:
		enum Level
		{
			debug = 0,
			information = 1,
			warning = 2,
			error = 3
		};

	public:
		log(Level lev = information) : ref(1), level(lev) {}
		~log();
		log(const log& other) : ref(other.ref + 1), level(other.level) {}

	private:
		quint32 ref;
		Level level;
};

/** @brief Global logging.
	@author vl@fidra.de
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT GlobalLog : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(GlobalLog)

 	friend class log;
 	friend log operator<<(log l, const QString& s);
 	friend log operator<<(log l, qint64 i);

	private:
		GlobalLog() : msg() {}

	signals:
		void newMessage(log::Level, const QString&);

	public:
		static GlobalLog* instance();

	private:
		void append(const QString& s) { msg += s; }
		void flush(log::Level level);

	private:
		QString msg;
};

inline log operator<<(log l, const QString& s)
{
	GlobalLog::instance()->append(s);
	return l;
}

inline log operator<<(log l, qint64 i)
{
	GlobalLog::instance()->append(QString::number(i));
	return l;
}

#endif
