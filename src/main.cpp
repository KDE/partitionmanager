/*
    SPDX-FileCopyrightText: 2008,2011 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2019 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/mainwindow.h"

#include <backend/corebackend.h>
#include <backend/corebackendmanager.h>

#include <util/helpers.h>
#include "util/guihelpers.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KMessageBox>
#include <KLocalizedString>

#include <config.h>
#include <utility>

std::pair<QString, QString> parseDevice(const QString& selectedDevice)
{
    QString device;
    QString partitionNr;
    if (selectedDevice.length()) {
        const bool isNvme = selectedDevice.startsWith(QStringLiteral("nvme"));
        // calculate the number of numbers on the last part of the info.
        int numSize = 0;
        for (int i = selectedDevice.length() - 1; i > 0; i--) {
            if (selectedDevice[i].isNumber()) {
                numSize += 1;
            } else {
                if (isNvme) {
                    // the entry is just an nvme without a partition.
                    if (selectedDevice[i] != QLatin1Char('p')) {
                        numSize = 0;
                        break;
                    }
                }
                break;
            }
        }
        partitionNr = selectedDevice.mid(selectedDevice.length() - numSize);

        if (numSize != 0) {
            // nvme has a `p` that also needs to be removed.
            if (isNvme) {
                numSize += 1;
            }

            device = selectedDevice.mid(0, selectedDevice.length() - numSize);
        } else {
            device = selectedDevice;
        }
    }
    return {device, partitionNr};
}

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
        xi18nc("@info:credit", "© 2008-2013 Volker Lanz\n© 2012-2020 Andrius Štikonas"));
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("partitionmanager"));

    aboutData.addAuthor(xi18nc("@info:credit", "Volker Lanz"), xi18nc("@info:credit", "Former maintainer"));
    aboutData.addAuthor(xi18nc("@info:credit", "Andrius Štikonas"), xi18nc("@info:credit", "Maintainer"), QStringLiteral("andrius@stikonas.eu"));
    aboutData.addCredit(xi18nc("@info:credit", "Teo Mrnjavac"), i18nc("@info:credit", "Former Calamares maintainer"), QStringLiteral("teo@kde.org"));
    aboutData.addCredit(xi18nc("@info:credit", "Chantara Tith"), i18nc("@info:credit", "LVM support"), QStringLiteral("tith.chantara@gmail.com"));
    aboutData.addCredit(xi18nc("@info:credit", "Pali Rohár"), i18nc("@info:credit", "UDF support"), QStringLiteral("pali.rohar@gmail.com"));
    aboutData.addCredit(i18n("Hugo Pereira Da Costa"), xi18nc("@info:credit", "Partition Widget Design"), QStringLiteral("hugo@oxygen-icons.org"));
    aboutData.addCredit(xi18nc("@info:credit", "Caio Jordão Carvalho"), i18nc("@info:credit", "Improved SMART support"), QStringLiteral("caiojcarvalho@gmail.com"));
    
    aboutData.setHomepage(QStringLiteral("https://www.kde.org/applications/system/partitionmanager"));

    KAboutData::setApplicationData(aboutData);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    QCommandLineOption deviceOption({QStringLiteral("device")}, xi18nc("@info:shell", "Device to manage"), QStringLiteral("device"));
    parser.addOption(deviceOption);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);

    registerMetaTypes();

    Config::instance(QStringLiteral("partitionmanagerrc"));

    if (!loadBackend())
        return 0;

    // device is the selected device minus the partition number.
    // we need all of them to select properly the things on screen.

    QString selectedDevice = parser.value(deviceOption);
    if (selectedDevice.startsWith(QStringLiteral("/dev/"))) {
        selectedDevice.remove(QStringLiteral("/dev/"));
    }
    auto [device, partitionNr] = parseDevice(selectedDevice);

    MainWindow* mainWindow = new MainWindow();
    QObject::connect(mainWindow, &MainWindow::scanFinished, mainWindow, [mainWindow, device = device, selectedDevice, partitionNr = partitionNr] {
        if (device.length()) {
            mainWindow->setCurrentDeviceByName(device);
            if (partitionNr.length()) {
                mainWindow->setCurrentPartitionByName(selectedDevice);
            }
        }
    });

    return app.exec();
}
