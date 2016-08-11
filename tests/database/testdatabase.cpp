/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * @date   2010-06-21
 * @brief  CLI test program for digiKam DB init
 *
 * @author Copyright (C) 2014-2016 by Gilles Caulier
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
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "coredbaccess.h"
#include "thumbsdbaccess.h"
#include "facedbaccess.h"
#include "dbengineparameters.h"
#include "scancontroller.h"
#include "setup.h"
#include "digikam_version.h"

using namespace Digikam;
using namespace FacesEngine;

const QString IMAGE_PATH(QFINDTESTDATA("data/testimages/"));

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

    DbEngineParameters params;
    params.databaseType = DbEngineParameters::SQLiteDatabaseType();
    params.setCoreDatabasePath(QDir::currentPath() + QLatin1String("/digikam-core-test.db"));
    params.setThumbsDatabasePath(QDir::currentPath() + QLatin1String("/digikam-thumbs-test.db"));
    params.setFaceDatabasePath(QDir::currentPath() + QLatin1String("/digikam-faces-test.db"));

    params.legacyAndDefaultChecks();

    // initialize database
    bool b = AlbumManager::instance()->setDatabase(params, false, IMAGE_PATH);

    qDebug() << "Database initialization done: " << b;

    QTimer::singleShot(500, &app, SLOT(quit()));
    app.exec();

    ScanController::instance()->shutDown();

    CoreDbAccess::cleanUpDatabase();
    ThumbsDbAccess::cleanUpDatabase();
    FaceDbAccess::cleanUpDatabase();

    return 0;
}
