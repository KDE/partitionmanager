/*************************************************************************
 *  Copyright (C) 2008-2010 by Volker Lanz <vl@fidra.de>                 *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "util/guihelpers.h"

#include <backend/corebackendmanager.h>

#include <QApplication>
#include <QFileInfo>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QStandardPaths>
#include <QString>

#include <KLocalizedString>
#include <KMessageBox>

#include <unistd.h>
#include <signal.h>

#include <config.h>

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

bool checkPermissions()
{
    if (geteuid() != 0) {
        // only try to gain root privileges if we have a valid (kde|gk)su(do) command and
        // we did not try so before: the dontsu-option is there to make sure there are no
        // endless loops of calling the same non-working (kde|gk)su(do) binary again and again.
        if (!suCommand().isEmpty() && !QCoreApplication::arguments().contains(QLatin1String("--dontsu"))) {
            QString argList;

            const QString suCmd = suCommand();

            // kdesu broke backward compatibility at some point and now only works with "-c";
            // kdesudo accepts either (with or without "-c"), but the gk* helpers only work
            // without. kdesu maintainers won't fix their app, so we need to work around that here.
            if (suCmd.indexOf(QStringLiteral("kdesu")) != -1)
                argList = QStringLiteral("-c ");

            argList += QCoreApplication::arguments().join(QStringLiteral(" ")) + QStringLiteral(" --dontsu");

            if (QProcess::execute(suCmd, QStringList(argList)) == 0)
                return false;
        }

        return KMessageBox::warningContinueCancel(nullptr, xi18nc("@info",
                "<para><warning>You do not have administrative privileges.</warning></para>"
                "<para>It is possible to run <application>%1</application> without these privileges. "
                "You will, however, <emphasis>not</emphasis> be allowed to apply operations.</para>"
                "<para>Do you want to continue running <application>%1</application>?</para>",
                QGuiApplication::applicationDisplayName()),
                xi18nc("@title:window", "No administrative privileges"),
                KGuiItem(xi18nc("@action:button", "Run without administrative privileges"), QStringLiteral("arrow-right")),
                KStandardGuiItem::cancel(),
                QStringLiteral("runWithoutRootPrivileges")) == KMessageBox::Continue;
    }

    return true;
}

bool loadBackend()
{
    if (CoreBackendManager::self()->load(Config::backend()) == false) {
        if (CoreBackendManager::self()->load(CoreBackendManager::defaultBackendName())) {
            KMessageBox::sorry(nullptr,
                               xi18nc("@info", "<para>The configured backend plugin \"%1\" could not be loaded.</para>"
                                      "<para>Loading the default backend plugin \"%2\" instead.</para>",
                                      Config::backend(), CoreBackendManager::defaultBackendName()),
                               xi18nc("@title:window", "Error: Could Not Load Backend Plugin"));
            Config::setBackend(CoreBackendManager::defaultBackendName());
        } else {
            KMessageBox::error(nullptr,
                               xi18nc("@info", "<para>Neither the configured (\"%1\") nor the default (\"%2\") backend "
                                      "plugin could be loaded.</para><para>Please check your installation.</para>",
                                      Config::backend(), CoreBackendManager::defaultBackendName()),
                               xi18nc("@title:window", "Error: Could Not Load Backend Plugin"));
            return false;
        }
    }

    return true;
}

QString suCommand()
{
    // First look for KF5 version of kdesu in libexec folder
    const QString candidates[] = { QStringLiteral(CMAKE_INSTALL_FULL_LIBEXECDIR_KF5"/kdesu"), QStringLiteral("kdesu"), QStringLiteral("kdesudo"), QStringLiteral("gksudo"), QStringLiteral("gksu") };
    QString rval;

    for (const auto &candidate : candidates) {
        rval = QStandardPaths::findExecutable(candidate);
        if (QFileInfo(rval).isExecutable())
            return rval;
    }

    return QString();
}

Capacity::Unit preferredUnit()
{
    return static_cast<Capacity::Unit>(Config::preferredUnit());
}

namespace GuiHelpers
{

FileSystem::Type defaultFileSystem()
{
    return static_cast<FileSystem::Type>(Config::defaultFileSystem());
}

std::array< QColor, FileSystem::__lastType > fileSystemColorCodesFromSettings()
{
    std::array< QColor, FileSystem::__lastType > cc;
    for (int i = 0; i < FileSystem::__lastType; ++i)
    {
        cc[ i ] = Config::fileSystemColorCode( i );
    }
    return cc;
}

}
