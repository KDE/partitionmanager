/***************************************************************************
 *   Copyright (C) 2008, 2009, 2010 by Volker Lanz <vl@fidra.de>           *
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

#include "backend/corebackendmanager.h"

#include "ops/operation.h"

#include <klocale.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kmenu.h>
#include <kstringhandler.h>

#include <solid/device.h>

#include <QProcess>
#include <QFileInfo>
#include <QApplication>
#include <QPainter>
#include <QIcon>
#include <QPixmap>
#include <QRect>
#include <QTreeWidget>
#include <QHeaderView>

#include <config.h>

#include <unistd.h>
#include <signal.h>

void registerMetaTypes()
{
	qRegisterMetaType<Operation*>("Operation*");
	qRegisterMetaType<Log::Level>("Log::Level");
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
		KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
		// only try to gain root privileges if we have a valid (kde|gk)su(do) command and
		// we did not try so before: the dontsu-option is there to make sure there are no
		// endless loops of calling the same non-working (kde|gk)su(do) binary again and again.
		if (!suCommand().isEmpty() && !args->isSet("dontsu"))
		{
			QStringList argList;

			const QString suCmd = suCommand();
			
			// kdesu broke backward compatibility at some point and now only works with "-c";
			// kdesudo accepts either (with or without "-c"), but the gk* helpers only work
			// without. kdesu maintainers won't fix their app, so we need to work around that here.
			if (suCmd.indexOf("kdesu") != -1)
				argList << "-c";

			argList << args->allArguments().join(" ") + " --dontsu";

			if (QProcess::execute(suCmd, argList) == 0)
				return false;
		}
	
		return KMessageBox::warningContinueCancel(NULL, i18nc("@info",
				"<para><warning>You do not have administrative privileges.</warning></para>"
				"<para>It is possible to run <application>%1</application> without these privileges. "
				"You will, however, <emphasis>not</emphasis> be allowed to apply operations.</para>"
				"<para>Do you want to continue running <application>%1</application>?</para>",
				KGlobal::mainComponent().aboutData()->programName()),
	 		i18nc("@title:window", "No administrative privileges"),
			KGuiItem(i18nc("@action:button", "Run without administrative privileges"), "arrow-right"),
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
		ki18nc("@info:credit", "(c) 2008, 2009, 2010, 2011 Volker Lanz")
	);

	about->addAuthor(ki18nc("@info:credit", "Volker Lanz"), KLocalizedString(), "vl@fidra.de");
	about->setHomepage("http://www.partitionmanager.org");

	about->addCredit(ki18n("Hugo Pereira Da Costa"), ki18nc("@info:credit", "Partition Widget Design"), "hugo@oxygen-icons.org");

	return about;
}

bool caseInsensitiveLessThan(const QString& s1, const QString& s2)
{
	return s1.toLower() < s2.toLower();
}

bool naturalLessThan(const QString& s1, const QString& s2)
{
    return KStringHandler::naturalCompare(s1, s2) < 0;
}

QIcon createFileSystemColor(FileSystem::Type type, quint32 size)
{
	QPixmap pixmap(size, size);
	QPainter painter(&pixmap);
	painter.setPen(QColor(0, 0, 0));
	painter.setBrush(Config::fileSystemColorCode(type));
	painter.drawRect(QRect(0, 0, pixmap.width() - 1, pixmap.height() - 1));
	painter.end();

	return QIcon(pixmap);
}

void showColumnsContextMenu(const QPoint& p, QTreeWidget& tree)
{
	KMenu headerMenu;

	headerMenu.addTitle(i18nc("@title:menu", "Columns"));

	QHeaderView* header = tree.header();

	for (qint32 i = 0; i < tree.model()->columnCount(); i++)
	{
		const int idx = header->logicalIndex(i);
		const QString text = tree.model()->headerData(idx, Qt::Horizontal).toString();

		QAction* action = headerMenu.addAction(text);
		action->setCheckable(true);
		action->setChecked(!header->isSectionHidden(idx));
		action->setData(idx);
		action->setEnabled(idx > 0);
	}

	QAction* action = headerMenu.exec(tree.header()->mapToGlobal(p));

	if (action != NULL)
	{
		const bool hidden = !action->isChecked();
		tree.setColumnHidden(action->data().toInt(), hidden);
		if (!hidden)
			tree.resizeColumnToContents(action->data().toInt());
	}
}

bool loadBackend()
{
	if (CoreBackendManager::self()->load(Config::backend()) == false)
	{
		if (CoreBackendManager::self()->load(CoreBackendManager::defaultBackendName()))
		{
			KMessageBox::sorry(NULL,
				i18nc("@info", "<para>The configured backend plugin \"%1\" could not be loaded.</para>"
					"<para>Loading the default backend plugin \"%2\" instead.</para>",
				Config::backend(), CoreBackendManager::defaultBackendName()),
				i18nc("@title:window", "Error: Could Not Load Backend Plugin"));
			Config::setBackend(CoreBackendManager::defaultBackendName());
		}
		else
		{
			KMessageBox::error(NULL,
				i18nc("@info", "<para>Neither the configured (\"%1\") nor the default (\"%2\") backend "
					"plugin could be loaded.</para><para>Please check your installation.</para>",
				Config::backend(), CoreBackendManager::defaultBackendName()),
				i18nc("@title:window", "Error: Could Not Load Backend Plugin"));
			return false;
		}
	}

	return true;
}

bool checkAccessibleDevices()
{
	if (getSolidDeviceList().empty())
	{
		KMessageBox::error(NULL,
			i18nc("@info", "<para>No usable devices could be found.</para><para>Make sure you have sufficient "
				"privileges to access block devices on your system.</para>"),
			i18nc("@title:window", "Error: No Usable Devices Found"));
		return false;
	}
	
	return true;
}

QList<Solid::Device> getSolidDeviceList()
{
	QString predicate = "StorageDrive.driveType == 'HardDisk'";

	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	if (args->count() > 0)
	{
		predicate = " [ " + predicate + " AND ";

		if (args->count() > 1)
			predicate += "[ ";

		for (qint32 i = 0; i < args->count(); i++)
		{
			predicate += QString("Block.device == '%1' ").arg(args->arg(i));

			if (i < args->count() - 1)
				predicate += "OR ";
		}

		if (args->count() > 1)
			predicate += "] ";

		predicate += ']';
	}

	return Solid::Device::listFromQuery(predicate);
}
