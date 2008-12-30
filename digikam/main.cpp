/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-07-28
 * Description : main program from digiKam
 *
 * Copyright (C) 2002-2006 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QSqlDatabase>

// KDE includes.

#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kimageio.h>
#include <ktip.h>
#include <kdeversion.h>
#include <kmessagebox.h>

// Libkexiv2 includes.

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "databaseparameters.h"
#include "scancontroller.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "digikamapp.h"
#include "digikamfirstrun.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData("digikam", 0,
                         ki18n("digiKam"),
                         digiKamVersion().toAscii(),
                         Digikam::digiKamSlogan(),
                         KAboutData::License_GPL,
                         Digikam::copyright(),
                         KLocalizedString(),
                         Digikam::webProjectUrl().url().toUtf8());


    Digikam::authorsRegistration(aboutData);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("download-from <path>", ki18n("Open camera dialog at <path>"));
    options.add("download-from-udi <udi>", ki18n("Open camera dialog for the device with Solid UDI <udi>"));
    options.add("detect-camera", ki18n("Automatically detect and open a connected gphoto2 camera"));
    options.add("database-directory <dir>", ki18n("Start digikam with the database file found in the directory <dir>"));
    KCmdLineArgs::addCmdLineOptions( options );

#if KEXIV2_VERSION >= 0x000300
    KExiv2Iface::KExiv2::initializeExiv2();
#endif

    KApplication app;

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("General Settings");
    QString version           = group.readEntry("Version", QString());

    group                     = config->group("Album Settings");
    QString dbPath            = group.readEntry("Database File Path", QString());
    QString albumPath         = group.readEntry("Album Path", QString());

    // 0.9 legacy
    if (dbPath.isEmpty() && !albumPath.isEmpty())
    {
        dbPath = albumPath;
        group.writeEntry("Database File Path", albumPath);
        group.sync();
    }

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    // TEMPORARY SOLUTION
    bool priorityDbPath = false;
    if (args && args->isSet("database-directory"))
    {
        priorityDbPath = true;
        dbPath = args->getOption("database-directory");
    }

    QFileInfo dirInfo(dbPath);

    // version 0.6 was the version when the new Albums Library
    // storage was implemented
    if (version.startsWith("0.5") ||
        !dirInfo.exists() ||
        !dirInfo.isDir())
    {
        // Run the first run
        Digikam::DigikamFirstRun *firstRun = new Digikam::DigikamFirstRun();
        app.setTopWidget(firstRun);
        if (firstRun->exec() == QDialog::Rejected)
            return 1;

        group     = config->group("Album Settings");
        dbPath    = group.readEntry("Database File Path", QString());
        albumPath = group.readEntry("Album Path", QString());
    }

    kDebug(50003) << "Root Album Path: " << albumPath << endl;
    kDebug(50003) << "Database Path: " << dbPath << endl;

    // Check if SQLite Qt4 plugin is available.

    if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        KMessageBox::errorList(0, i18n("Qt4 SQLite database plugin is not available. "
                                       "Please install it!\n"
                                       "Database plugins installed are:"),
                               QSqlDatabase::drivers());

        kDebug(50003) << "QT Sql drivers list: " << QSqlDatabase::drivers() << endl;
        return 1;
    }

    // initialize database
    Digikam::AlbumManager* man = Digikam::AlbumManager::instance();
    if (!man->setDatabase(dbPath, priorityDbPath))
        return 1;

    // ensure we have one album root
    if (Digikam::CollectionManager::instance()->allLocations().isEmpty())
    {
        Digikam::CollectionManager::instance()->addLocation(albumPath);
    }

    Digikam::DigikamApp *digikam = new Digikam::DigikamApp();

    app.setTopWidget(digikam);
    digikam->show();

    if (args && args->isSet("download-from"))
        digikam->downloadFrom(args->getOption("download-from"));
    else if (args && args->isSet("download-from-udi"))
        digikam->downloadFromUdi(args->getOption("download-from-udi"));
    else if (args && args->isSet("detect-camera"))
        digikam->autoDetect();

    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("kipi/tips");

    KGlobal::locale()->insertCatalog("kipiplugins");
    KGlobal::locale()->insertCatalog("libkdcraw");

    KTipDialog::showMultiTip(0, tipsFiles, false);

    int ret = app.exec();

#if KEXIV2_VERSION >= 0x000300
    KExiv2Iface::KExiv2::cleanupExiv2();
#endif

    return ret;
}
