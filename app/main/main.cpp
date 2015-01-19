/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-07-28
 * Description : main program from digiKam
 *
 * Copyright (C) 2002-2006 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QDBusConnection>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QMessageBox>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>
#include <ktip.h>
#include <kaboutdata.h>

// Libkexiv2 includes

#include <kexiv2.h>

// Local includes

#include "digikam_debug.h"
#include "dmessagebox.h"
#include "albummanager.h"
#include "assistantdlg.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "daboutdata.h"
#include "databaseaccess.h"
#include "databaseparameters.h"
#include "digikamapp.h"
#include "scancontroller.h"
#include "thumbnaildatabaseaccess.h"
#include "digikam_version.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("digikam");
    KLocalizedString::setApplicationDomain("libkdcraw");
    KLocalizedString::setApplicationDomain("libkexiv2");
    KLocalizedString::setApplicationDomain("libkface");
    KLocalizedString::setApplicationDomain("libkgeomap");
    KLocalizedString::setApplicationDomain("kipiplugins");
    KLocalizedString::setApplicationDomain("libkipi");

    KAboutData aboutData(QString::fromLatin1("digikam"), // component name
                         i18n("digiKam"),                // display name
                         digiKamVersion());

    aboutData.setShortDescription(DAboutData::digiKamSlogan());;
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(DAboutData::copyright());
    aboutData.setOtherText(additionalInformation());
    aboutData.setHomepage(DAboutData::webProjectUrl().url());

    DAboutData::authorsRegistration(aboutData);

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("download-from"),      i18n("Open camera dialog at <path>"),                                             QLatin1String("path")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("download-from-udi"),  i18n("Open camera dialog for the device with Solid UDI <udi>"),                   QLatin1String("udi")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("detect-camera"),      i18n("Automatically detect and open a connected gphoto2 camera")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("database-directory"), i18n("Start digikam with the SQLite database file found in the directory <dir>"), QLatin1String("dir")));

    KExiv2Iface::KExiv2::initializeExiv2();

    // Check if Qt database plugins are available.

    if (!QSqlDatabase::isDriverAvailable(DatabaseParameters::SQLiteDatabaseType()) &&
        !QSqlDatabase::isDriverAvailable(DatabaseParameters::MySQLDatabaseType()))
    {
        if (QSqlDatabase::drivers().isEmpty())
        {
            QMessageBox::critical(qApp->activeWindow(),
                                  qApp->applicationName(),
                                  i18n("Run-time Qt SQLite or MySQL database plugin is not available. "
                                       "please install it.\n"
                                       "There is no database plugin installed on your computer."));
        }
        else
        {
            DMessageBox::showInformationList(QMessageBox::Warning,
                                             qApp->activeWindow(),
                                             qApp->applicationName(),
                                             i18n("Run-time Qt SQLite or MySQL database plugin is not available. "
                                                  "Please install it.\n"
                                                  "Database plugins installed on your computer are listed below."),
                                             QSqlDatabase::drivers());
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "QT Sql drivers list: " << QSqlDatabase::drivers();
        return 1;
    }

    QString commandLineDBPath;

    if (parser.isSet("database-directory"))
    {
        QFileInfo commandLineDBDir(parser.value("database-directory"));

        if (!commandLineDBDir.exists() || !commandLineDBDir.isDir())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "The given database-directory does not exist or is not readable. Ignoring." << commandLineDBDir.path();
        }
        else
        {
            commandLineDBPath = commandLineDBDir.path();
        }
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("General Settings");
    QString version           = group.readEntry("Version", QString());
    KConfigGroup mainConfig   = config->group("Album Settings");

    QString            firstAlbumPath;
    DatabaseParameters params;

    // Run the first run assistant if we have no or very old config
    if (!mainConfig.exists() || (version.startsWith(QLatin1String("0.5"))))
    {
        AssistantDlg firstRun;
        firstRun.show();

        if (firstRun.exec() == QDialog::Rejected)
        {
            return 1;
        }

        // parameters are written to config
        firstAlbumPath = firstRun.firstAlbumPath();
        AlbumManager::checkDatabaseDirsAfterFirstRun(firstRun.databasePath(), firstAlbumPath);
    }

    if (!commandLineDBPath.isNull())
    {
        // command line option set?
        params = DatabaseParameters::parametersForSQLiteDefaultFile(commandLineDBPath);
    }
    else
    {
        params = DatabaseParameters::parametersFromConfig(config);
        params.legacyAndDefaultChecks(firstAlbumPath);
        // sync to config, for all first-run or upgrade situations
        params.writeToConfig(config);
    }

    /*
     * Register a dummy service on dbus.
     * This is needed for the internal database server, which checks if at least one
     * digikam instance is running on dbus.
     * The first real dbus instance is registered within the DigikamApp() constructor,
     * so we create a service on dbus which is unregistered after initialization the application.
     */
    QDBusConnection::sessionBus().registerService("org.kde.digikam.startup-" +
                     QString::number(QCoreApplication::instance()->applicationPid()));

    // initialize database
    AlbumManager::instance()->setDatabase(params, !commandLineDBPath.isNull(), firstAlbumPath);

    // create main window
    DigikamApp* const digikam = new DigikamApp();

    // Bug #247175:
    // Add a connection to the destroyed() signal when the digiKam mainwindow has been
    // closed. This should prevent digiKam from staying open in the background.
    //
    // Right now this is the easiest and cleanest fix for the described problem, but we might re-think the
    // solution later on, just in case there are better ways to do it.
    QObject::connect(digikam, SIGNAL(destroyed(QObject*)),
                     &app, SLOT(quit()));

    // Unregister the dummy service
    QDBusConnection::sessionBus().unregisterService("org.kde.digikam.startup-" +
                     QString::number(QCoreApplication::instance()->applicationPid()));

    digikam->restoreSession();
    digikam->show();

    if (parser.isSet("download-from"))
    {
        digikam->downloadFrom(parser.value("download-from"));
    }
    else if (parser.isSet("download-from-udi"))
    {
        digikam->downloadFromUdi(parser.value("download-from-udi"));
    }
    else if (parser.isSet("detect-camera"))
    {
        digikam->autoDetect();
    }

    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("kipi/tips");

    if (!app.isSessionRestored())
    {
        KTipDialog::showMultiTip(digikam, tipsFiles, false);
    }

    int ret = app.exec();

    DatabaseAccess::cleanUpDatabase();
    ThumbnailDatabaseAccess::cleanUpDatabase();
    KExiv2Iface::KExiv2::cleanupExiv2();

    return ret;
}
