/*************************************************************************
 *  Copyright (C) 2008,2011 by Volker Lanz <vl@fidra.de>                 *
 *  Copyright (C) 2014-2018 by Andrius Štikonas <andrius@stikonas.eu>    *
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

#include "gui/mainwindow.h"

#include <backend/corebackend.h>
#include <backend/corebackendmanager.h>

#include <util/helpers.h>
#include "util/guihelpers.h"

#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KMessageBox>
#include <KLocalizedString>

#include <config.h>

int Q_DECL_IMPORT main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("partitionmanager")));

    KLocalizedString::setApplicationDomain("partitionmanager");
    KAboutData aboutData (
        QStringLiteral("partitionmanager"),
        xi18nc("@title", "<application>KDE Partition Manager</application>"),
        QStringLiteral(VERSION),
        xi18nc("@title", "Manage your disks, partitions and file systems"),
        KAboutLicense::GPL_V3,
        xi18nc("@info:credit", "© 2008-2013 Volker Lanz\n© 2012-2018 Andrius Štikonas"));
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("partitionmanager"));

    aboutData.addAuthor(xi18nc("@info:credit", "Volker Lanz"), xi18nc("@info:credit", "Former maintainer"));
    aboutData.addAuthor(xi18nc("@info:credit", "Andrius Štikonas"), xi18nc("@info:credit", "Maintainer"), QStringLiteral("andrius@stikonas.eu"));
    aboutData.addCredit(xi18nc("@info:credit", "Teo Mrnjavac"), i18nc("@info:credit", "Former Calamares maintainer"), QStringLiteral("teo@kde.org"));
    aboutData.addCredit(xi18nc("@info:credit", "Chantara Tith"), i18nc("@info:credit", "LVM support"), QStringLiteral("tith.chantara@gmail.com"));
    aboutData.addCredit(xi18nc("@info:credit", "Pali Rohár"), i18nc("@info:credit", "UDF support"), QStringLiteral("pali.rohar@gmail.com"));
    aboutData.setHomepage(QStringLiteral("https://www.kde.org/applications/system/kdepartitionmanager"));

    aboutData.addCredit(i18n("Hugo Pereira Da Costa"), xi18nc("@info:credit", "Partition Widget Design"), QStringLiteral("hugo@oxygen-icons.org"));
    KAboutData::setApplicationData(aboutData);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
// FIXME    parser.addPositionalArgument(QStringLiteral("device"), xi18nc("@info:shell", "Device(s) to manage"), QStringLiteral("[device...]"));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);

    registerMetaTypes();

    Config::instance(QStringLiteral("partitionmanagerrc"));

    if (!loadBackend())
        return 0;

    MainWindow* mainWindow = new MainWindow();
    Q_UNUSED(mainWindow)

    return app.exec();
}
