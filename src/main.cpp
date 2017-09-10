/*************************************************************************
 *  Copyright (C) 2008,2011 by Volker Lanz <vl@fidra.de>                 *
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
#include <Kdelibs4ConfigMigrator>
#include <KMessageBox>
#include <KLocalizedString>

#include <config.h>

int Q_DECL_IMPORT main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Kdelibs4ConfigMigrator migrate(QLatin1Literal("partitionmanager"));
    migrate.setConfigFiles({ QLatin1Literal("partitionmanagerrc") });
    migrate.setUiFiles({ QStringLiteral("partitionmanagerui.rc") });
    migrate.migrate();

    KLocalizedString::setApplicationDomain("partitionmanager");
    KAboutData aboutData (
        QStringLiteral("partitionmanager"),
        xi18nc("@title", "<application>KDE Partition Manager</application>"),
        QStringLiteral(VERSION),
        xi18nc("@title", "Manage your disks, partitions and file systems"),
        KAboutLicense::GPL_V3,
        xi18nc("@info:credit", "© 2008-2013 Volker Lanz\n© 2012-2017 Andrius Štikonas"));
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("partitionmanager"));

    aboutData.addAuthor(xi18nc("@info:credit", "Volker Lanz"), xi18nc("@info:credit", "Former maintainer"));
    aboutData.addAuthor(xi18nc("@info:credit", "Andrius Štikonas"), xi18nc("@info:credit", "Maintainer"), QStringLiteral("andrius@stikonas.eu"));
    aboutData.addAuthor(xi18nc("@info:credit", "Teo Mrnjavac"), i18nc("@info:credit", "Calamares maintainer"), QStringLiteral("teo@kde.org"));
    aboutData.addAuthor(xi18nc("@info:credit", "Chantara Tith"), i18nc("@info:credit", "LVM support"), QStringLiteral("tith.chantara@gmail.com"));
    aboutData.addAuthor(xi18nc("@info:credit", "Pali Rohár"), i18nc("@info:credit", "UDF support"), QStringLiteral("pali.rohar@gmail.com"));
    aboutData.setHomepage(QStringLiteral("https://www.kde.org/applications/system/kdepartitionmanager"));

    aboutData.addCredit(i18n("Hugo Pereira Da Costa"), xi18nc("@info:credit", "Partition Widget Design"), QStringLiteral("hugo@oxygen-icons.org"));
    KAboutData::setApplicationData(aboutData);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    KCrash::initialize();

    QCommandLineParser parser;
    parser.setApplicationDescription(aboutData.shortDescription());
    parser.addHelpOption();
    parser.addVersionOption();
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QLatin1Literal("dontsu"), xi18nc("@info:shell", "Do not try to gain super user privileges")));
    parser.addOption(QCommandLineOption(QLatin1Literal("advconfig"), xi18nc("@info:shell", "Show advanced tab in configuration dialog")));
    parser.addPositionalArgument(QStringLiteral("device"), xi18nc("@info:shell", "Device(s) to manage"), QStringLiteral("[device...]"));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    registerMetaTypes();
    if (!checkPermissions())
        return 0;

    Config::instance(QStringLiteral("partitionmanagerrc"));

    if (!loadBackend())
        return 0;

    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

    return app.exec();
}
