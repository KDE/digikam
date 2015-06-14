/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-07-28
 * Description : main program from digiKam
 *
 * Copyright (C) 2002-2006 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QDBusConnection>
#include <QString>
#include <QStringList>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kimageio.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktip.h>

// Libkexiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Local includes

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
#include "version.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    KAboutData aboutData("digikam",
                         0,
                         ki18n("digiKam"),
                         digiKamVersion().toAscii(),
                         DAboutData::digiKamSlogan(),
                         KAboutData::License_GPL,
                         DAboutData::copyright(),
                         additionalInformation(),
                         DAboutData::webProjectUrl().url().toUtf8());

    DAboutData::authorsRegistration(aboutData);

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("download-from <path>",     ki18n("Open camera dialog at <path>"));
    options.add("download-from-udi <udi>",  ki18n("Open camera dialog for the device with Solid UDI <udi>"));
    options.add("detect-camera",            ki18n("Automatically detect and open a connected gphoto2 camera"));
    options.add("database-directory <dir>", ki18n("Start digikam with the SQLite database file found in the directory <dir>"));
    KCmdLineArgs::addCmdLineOptions(options);

    KExiv2Iface::KExiv2::initializeExiv2();

    KApplication app;

    // Check if SQLite Qt4 plugin is available.

    if (!QSqlDatabase::isDriverAvailable(DatabaseParameters::SQLiteDatabaseType()) &&
        !QSqlDatabase::isDriverAvailable(DatabaseParameters::MySQLDatabaseType()))
    {
        if (QSqlDatabase::drivers().isEmpty())
        {
            KMessageBox::error(0, i18n("Run-time Qt4 SQLite or MySQL database plugin is not available - "
                                       "please install it.\n"
                                       "There is no database plugin installed on your computer."));
        }
        else
        {
            KMessageBox::errorList(0, i18n("Run-time Qt4 SQLite or MySQL database plugin is not available - "
                                           "please install it.\n"
                                           "Database plugins installed on your computer are listed below:"),
                                   QSqlDatabase::drivers());
        }

        kDebug() << "QT Sql drivers list: " << QSqlDatabase::drivers();
        return 1;
    }

    KCmdLineArgs* const args = KCmdLineArgs::parsedArgs();

    QString commandLineDBPath;

    if (args && args->isSet("database-directory"))
    {
        QFileInfo commandLineDBDir(args->getOption("database-directory"));

        if (!commandLineDBDir.exists() || !commandLineDBDir.isDir())
        {
            kError() << "The given database-directory does not exist or is not readable. Ignoring." << commandLineDBDir.path();
        }
        else
        {
            commandLineDBPath = commandLineDBDir.path();
        }
    }

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("General Settings");
    QString version           = group.readEntry("Version", QString());
    KConfigGroup mainConfig   = config->group("Album Settings");

    QString            firstAlbumPath;
    DatabaseParameters params;

    // Run the first run assistant if we have no or very old config
    if (!mainConfig.exists() || (version.startsWith(QLatin1String("0.5"))))
    {
        AssistantDlg firstRun;
        app.setTopWidget(&firstRun);
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


    app.setTopWidget(digikam);
    digikam->restoreSession();
    digikam->show();

    if (args && args->isSet("download-from"))
    {
        digikam->downloadFrom(args->getOption("download-from"));
    }
    else if (args && args->isSet("download-from-udi"))
    {
        digikam->downloadFromUdi(args->getOption("download-from-udi"));
    }
    else if (args && args->isSet("detect-camera"))
    {
        digikam->autoDetect();
    }

    KGlobal::locale()->insertCatalog("kipiplugins");
    KGlobal::locale()->insertCatalog("libkdcraw");
    KGlobal::locale()->insertCatalog("libkexiv2");
    KGlobal::locale()->insertCatalog("libkipi");

    KTipDialog::setShowOnStart(false);

    int ret = app.exec();

    DatabaseAccess::cleanUpDatabase();
    ThumbnailDatabaseAccess::cleanUpDatabase();
    KExiv2Iface::KExiv2::cleanupExiv2();

    return ret;
}
