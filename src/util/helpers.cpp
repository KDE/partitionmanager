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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "util/helpers.h"
#include "util/globallog.h"

#include "ops/operation.h"

#include <klocale.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <QProcess>
#include <QFileInfo>
#include <QApplication>

#include <unistd.h>
#include <signal.h>

void registerMetaTypes()
{
	qRegisterMetaType<Operation*>("Operation*");
	qRegisterMetaType<log::Level>("log::Level");
}

void unblockSigChild()
{
	sigset_t sset;
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_UNBLOCK, &sset, NULL);
}

static QString suCommand()
{
	KStandardDirs d;
	const char* candidates[] = { "kdesu", "kdesudo", "gksudo", "gksu" };
	QString rval;

	for (quint32 i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++)
	{
		rval = d.locate("exe", candidates[i]);
		if (QFileInfo(rval).isExecutable())
			return rval;
	}

	return QString();
}

bool checkPermissions()
{
	if (geteuid() != 0)
	{
		// only try to gain root privileges if we have a valid (kde|gk)su(do) command and 
		// we did not try so before: the dontsu-option is there to make sure there are no 
		// endless loops of calling the same non-working (kde|gk)su(do) binary again and again.
		if (!suCommand().isEmpty() && !KCmdLineArgs::parsedArgs()->isSet("dontsu"))
		{
			QStringList args = qApp->arguments();

			// first argument is our own command again (i.e., argv[0])
			if (!args.isEmpty())
				args.removeFirst();
		
			// arguments to partition manager must be _one_ argument to (kde|gk)su(do)
			const QString suArgs = qApp->applicationFilePath() + args.join(" ") + " --dontsu";
			if (QProcess::execute(suCommand(), QStringList() << suArgs) == 0)
				return false;
		}
		
		return KMessageBox::warningContinueCancel(NULL, i18nc("@info",
				"<para><warning>You do not have administrative privileges.</warning></para>"
				"<para>It is possible to run <application>%1</application> without these privileges. "
				"You will, however, <emphasis>not</emphasis> be allowed to apply operations.</para>"
				"<para>Do you want to continue running <application>%1</application>?</para>",
				KGlobal::mainComponent().aboutData()->programName()),
	 		i18nc("@title:window", "No administrative privileges"),
			KGuiItem(i18nc("@action:button", "Run without administrative privileges")),
			KStandardGuiItem::cancel(),
			"runWithoutRootPrivileges") == KMessageBox::Continue;
	}

	return true;
}

KAboutData* createPartitionManagerAboutData()
{
	KAboutData* about = new KAboutData(
		"partitionmanager",
		NULL,
		ki18nc("@title", "<application>KDE Partition Manager</application>"),
		VERSION,
		ki18nc("@title", "Manage your disks, partitions and file systems"),
		KAboutData::License_GPL,
		ki18nc("@info:credit", "(c) 2008, 2009 Volker Lanz")
	);

	about->addAuthor(ki18nc("@info:credit", "Volker Lanz"), KLocalizedString(), "vl@fidra.de");
	about->setHomepage("http://www.partitionmanager.org");

	return about;
}

bool caseInsensitiveLessThan(const QString& s1, const QString& s2)
{
	return s1.toLower() < s2.toLower();
}
