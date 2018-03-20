/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : CLI test program for switch digiKam DB from sqlite to mysql
 *
 * Copyright (C) 2014-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
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

#include <QApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QString>
#include <QTimer>
#include <QTest>
#include <QCommandLineParser>
#include <QDebug>

// KDE includes

#include <kaboutdata.h>

// Local includes

#include "daboutdata.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "thumbsdbaccess.h"
#include "facedbaccess.h"
#include "dbengineparameters.h"
#include "scancontroller.h"
#include "digikam_version.h"

using namespace Digikam;


const QString IMAGE_PATH(QFINDTESTDATA("data/testimages/"));

int main(int argc, char** argv)
{

    if (argc != 2)
    {
        qDebug() << "testdatabaseswitch - test database switch";
        qDebug() << "Usage: <sqliteToMysql | mysqlToSqlite>";
        return -1;
    }

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
    QString switchCondition = QString::fromLatin1(argv[1]);

    // ------------------------------------------------------------------------------------

    if (switchCondition == QLatin1String("sqliteToMysql"))
    {
        qDebug() << "Setup sqlite Database...";

        DbEngineParameters params;

        params.databaseType = DbEngineParameters::SQLiteDatabaseType();
        params.setCoreDatabasePath(QDir::currentPath() + QLatin1String("/digikam-core-test.db"));
        params.setThumbsDatabasePath(QDir::currentPath() + QLatin1String("/digikam-thumbs-test.db"));
        params.setFaceDatabasePath(QDir::currentPath() + QLatin1String("/digikam-faces-test.db"));
        params.legacyAndDefaultChecks();

        qDebug() << "Initializing database...";
        bool b = AlbumManager::instance()->setDatabase(params, false, IMAGE_PATH);

        qDebug() << "Database initialization done: " << b;

        QTest::qWait(3000);

         //qDebug() << "Shutting down database";
         //ScanController::instance()->shutDown();
         //AlbumManager::instance()->cleanUp();

        qDebug() << "Cleaning DB now";
        CoreDbAccess::cleanUpDatabase();
        ThumbsDbAccess::cleanUpDatabase();
        FaceDbAccess::cleanUpDatabase();

        // ------------------------------------------------------------------------------------

        qDebug() << "Setup mysql Database...";

        params                            = DbEngineParameters();
        QString defaultAkDir              = DbEngineParameters::internalServerPrivatePath();
        QString miscDir                   = QDir(defaultAkDir).absoluteFilePath(QLatin1String("db_misc"));
        params.databaseType               = DbEngineParameters::MySQLDatabaseType();
        params.databaseNameCore           = QLatin1String("digikam");
        params.databaseNameThumbnails     = QLatin1String("digikam");
        params.databaseNameFace           = QLatin1String("digikam");
        params.userName                   = QLatin1String("root");
        params.password                   = QString();
        params.internalServer             = true;
        params.internalServerDBPath       = QDir::currentPath();
        params.internalServerMysqlServCmd = DbEngineParameters::defaultMysqlServerCmd();
        params.internalServerMysqlInitCmd = DbEngineParameters::defaultMysqlInitCmd();
        params.hostName                   = QString();
        params.port                       = -1;
        params.connectOptions             = QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir);

        // ------------------------------------------------------------------------------------

        qDebug() << "Initializing database...";
        AlbumManager::instance()->changeDatabase(params);

        qDebug() << "Database switch done";

        QTimer::singleShot(5000, &app, SLOT(quit()));
        app.exec();

        qDebug() << "Shutting down database";
        ScanController::instance()->shutDown();
        AlbumManager::instance()->cleanUp();

        qDebug() << "Cleaning DB now";
        CoreDbAccess::cleanUpDatabase();
        ThumbsDbAccess::cleanUpDatabase();
        FaceDbAccess::cleanUpDatabase();
    }

    else if(switchCondition == QLatin1String("mysqlToSqlite"))
    {
        DbEngineParameters params;

        qDebug() << "Setup mysql Database...";

        QString defaultAkDir              = DbEngineParameters::internalServerPrivatePath();
        QString miscDir                   = QDir(defaultAkDir).absoluteFilePath(QLatin1String("db_misc"));
        params.databaseType               = DbEngineParameters::MySQLDatabaseType();
        params.databaseNameCore           = QLatin1String("digikam");
        params.databaseNameThumbnails     = QLatin1String("digikam");
        params.databaseNameFace           = QLatin1String("digikam");
        params.userName                   = QLatin1String("root");
        params.password                   = QString();
        params.internalServer             = true;
        params.internalServerDBPath       = QDir::currentPath();
        params.internalServerMysqlServCmd = DbEngineParameters::defaultMysqlServerCmd();
        params.internalServerMysqlInitCmd = DbEngineParameters::defaultMysqlInitCmd();
        params.hostName                   = QString();
        params.port                       = -1;
        params.connectOptions             = QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir);

        // ------------------------------------------------------------------------------------

        qDebug() << "Initializing database...";
        AlbumManager::instance()->changeDatabase(params);

        qDebug() << "Database switch done";

        QTimer::singleShot(5000, &app, SLOT(quit()));
        app.exec();

        qDebug() << "Shutting down database";
        ScanController::instance()->shutDown();
        AlbumManager::instance()->cleanUp();

        qDebug() << "Cleaning DB now";
        CoreDbAccess::cleanUpDatabase();
        ThumbsDbAccess::cleanUpDatabase();
        FaceDbAccess::cleanUpDatabase();

        // ------------------------------------------------------------------------------------

        qDebug() << "Setup sqlite Database...";

        params.databaseType = DbEngineParameters::SQLiteDatabaseType();
        params.setCoreDatabasePath(QDir::currentPath() + QLatin1String("/digikam-core-test.db"));
        params.setThumbsDatabasePath(QDir::currentPath() + QLatin1String("/digikam-thumbs-test.db"));
        params.setFaceDatabasePath(QDir::currentPath() + QLatin1String("/digikam-faces-test.db"));
        params.legacyAndDefaultChecks();

        qDebug() << "Initializing database...";
        bool b = AlbumManager::instance()->setDatabase(params, false, IMAGE_PATH);

        qDebug() << "Database initialization done: " << b;

        QTest::qWait(3000);

        //qDebug() << "Shutting down database";
        //ScanController::instance()->shutDown();
        //AlbumManager::instance()->cleanUp();

        qDebug() << "Cleaning DB now";
        CoreDbAccess::cleanUpDatabase();
        ThumbsDbAccess::cleanUpDatabase();
        FaceDbAccess::cleanUpDatabase();
    }

    return 0;
}
