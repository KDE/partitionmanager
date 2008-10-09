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

#include "util/externalcommand.h"

#include "util/report.h"

#include <QString>
#include <QStringList>

#include <klocale.h>
#include <kdebug.h>

#include <stdlib.h>

/** Creates a new ExternalCommand instance without Report.
	@param cmd the command to run
	@param args the arguments to pass to the command
*/
ExternalCommand::ExternalCommand(const QString& cmd, const QStringList& args) :
	QProcess(),
	m_Report(NULL),
	m_Command(cmd),
	m_Args(args),
	m_ExitCode(-1),
	m_Output()
{
	setup();
}

/** Creates a new ExternalCommand instance with Report.
	@param report the Report to write output to.
	@param cmd the command to run
	@param args the arguments to pass to the command
 */
ExternalCommand::ExternalCommand(Report& report, const QString& cmd, const QStringList& args) :
	QProcess(),
	m_Report(report.newChild()),
	m_Command(cmd),
	m_Args(args),
	m_ExitCode(-1),
	m_Output()
{
	setup();
}

void ExternalCommand::setup()
{
	setEnvironment(QStringList() << "LC_ALL=C" << QString("PATH=") + getenv("PATH"));
	setProcessChannelMode(MergedChannels);

	connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(onFinished(int)));
	connect(this, SIGNAL(readyReadStandardOutput()), SLOT(onReadOutput()));
}

/** Starts the external command.
	@param timeout timeout to wait for the process to start
	@return true on success
*/
bool ExternalCommand::start(int timeout)
{
	QProcess::start(command(), args());

	if (report())
		report()->setCommand(i18nc("@info/plain", "Command: %1 %2", command(), args().join(" ")));
	
	if (!waitForStarted(timeout))
	{
		if (report())
			report()->line() << i18nc("@info/plain", "(Command timeout while starting)");
		return false;
	}

	return true;
}

/** Waits for the external command to finish.
	@param timeout timeout to wait until the process finishes.
	@return true on success
*/
bool ExternalCommand::waitFor(int timeout)
{
	closeWriteChannel();

	if (!waitForFinished(timeout))
	{
		if (report())
			report()->line() << i18nc("@info/plain", "(Command timeout while running)");
		return false;
	}

	onReadOutput();
	return true;
}

/** Runs the command.
	@param timeout timeout to use for waiting when starting and when waiting for the process to finish
	@return true on success
*/
bool ExternalCommand::run(int timeout)
{
	return start(timeout) && waitFor(timeout) && exitStatus() == 0;
}

void ExternalCommand::onReadOutput()
{
	const QString s = QString(readAllStandardOutput());
	
	m_Output += s;

	if (report())
		*report() << s;
}

void ExternalCommand::onFinished(int exitCode)
{
	setExitCode(exitCode);
}
