/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * @date   2010-06-21
 * @brief  CLI test program for digiKam DB init
 *
 * @author Copyright (C) 2014-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QDBusConnection>
#include <QString>
#include <QTimer>
#include <QCommandLineParser>
#include <QDebug>

// digiKam includes

#include "daboutdata.h"
#include "albummanager.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "coredbaccess.h"
#include "databaseparameters.h"
#include "scancontroller.h"
#include "setup.h"
#include "thumbsdbaccess.h"
#include "digikam_version.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    KAboutData aboutData(QString::fromLatin1("digikam"),
                         QString::fromLatin1("digiKam"), // No need i18n here.
                         digiKamVersion());
    
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    DatabaseParameters params;
    params.databaseType = DatabaseParameters::SQLiteDatabaseType();
    params.setDatabasePath(QDir::currentPath() + QLatin1String("/digikam-test.db"));
    params.setThumbsDatabasePath(QDir::currentPath() + QLatin1String("/digikam-thumbs-test.db"));

    params.legacyAndDefaultChecks();

    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.digikam.startup-") +
                     QString::number(QCoreApplication::instance()->applicationPid()));

    // initialize database
    bool b = AlbumManager::instance()->setDatabase(params, false, QLatin1String("/media/fotos/Digikam Sample/"));

    qDebug() << "Database initialization done: " << b;

    QTimer::singleShot(500, &app, SLOT(quit()));
    app.exec();

    ScanController::instance()->shutDown();

    CoreDbAccess::cleanUpDatabase();
    ThumbsDbAccess::cleanUpDatabase();

    return 0;
}
