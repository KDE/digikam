/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Face database schema updater
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "facedbschemaupdater.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dbenginebackend.h"
#include "facedbaccess.h"
#include "facedb.h"

namespace Digikam
{

int FaceDbSchemaUpdater::schemaVersion()
{
    return 3;
}

// -------------------------------------------------------------------------------------

class Q_DECL_HIDDEN FaceDbSchemaUpdater::Private
{
public:

    explicit Private()
        : setError(false),
          currentVersion(0),
          currentRequiredVersion(0),
          dbAccess(0),
          observer(0)
    {
    }

    bool                    setError;

    int                     currentVersion;
    int                     currentRequiredVersion;

    FaceDbAccess*           dbAccess;

    InitializationObserver* observer;
};

FaceDbSchemaUpdater::FaceDbSchemaUpdater(FaceDbAccess* const dbAccess)
    : d(new Private)
{
    d->dbAccess = dbAccess;
}

FaceDbSchemaUpdater::~FaceDbSchemaUpdater()
{
    delete d;
}

void FaceDbSchemaUpdater::setObserver(InitializationObserver* const observer)
{
    d->observer = observer;
}

bool FaceDbSchemaUpdater::update()
{
    bool success = startUpdates();

    // even on failure, try to set current version - it may have incremented
    if (d->currentVersion)
    {
        d->dbAccess->db()->setSetting(QLatin1String("DBFaceVersion"), QString::number(d->currentVersion));
    }

    if (d->currentRequiredVersion)
    {
        d->dbAccess->db()->setSetting(QLatin1String("DBFaceVersionRequired"), QString::number(d->currentRequiredVersion));
    }

    return success;
}

bool FaceDbSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = d->dbAccess->backend()->tables();

    if (tables.contains(QLatin1String("Identities"), Qt::CaseInsensitive))
    {
        // Find out schema version of db file
        QString version         = d->dbAccess->db()->setting(QLatin1String("DBFaceVersion"));
        QString versionRequired = d->dbAccess->db()->setting(QLatin1String("DBFaceVersionRequired"));
        qCDebug(DIGIKAM_FACEDB_LOG) << "Face database: have a structure version " << version;

        // mini schema update
        if (version.isEmpty() && d->dbAccess->parameters().isSQLite())
        {
            version = d->dbAccess->db()->setting(QLatin1String("DBVersion"));
        }

        // We absolutely require the DBFaceVersion setting
        if (version.isEmpty())
        {
            // Something is damaged. Give up.
            qCWarning(DIGIKAM_FACEDB_LOG) << "DBFaceVersion not available! Giving up schema upgrading.";

            QString errorMsg = i18n("The database is not valid: "
                                    "the \"DBFaceVersion\" setting does not exist. "
                                    "The current database schema version cannot be verified. "
                                    "Try to start with an empty database. ");

            d->dbAccess->setLastError(errorMsg);

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        // current version describes the current state of the schema in the db,
        // schemaVersion is the version required by the program.
        d->currentVersion = version.toInt();

        if (d->currentVersion > schemaVersion())
        {
            // trying to open a database with a more advanced than this FaceDbSchemaUpdater supports
            if (!versionRequired.isEmpty() && versionRequired.toInt() <= schemaVersion())
            {
                // version required may be less than current version
                return true;
            }
            else
            {
                QString errorMsg = i18n("The database has been used with a more recent version of digiKam "
                                        "and has been updated to a database schema which cannot be used with this version. "
                                        "(This means this digiKam version is too old, or the database format is to recent.) "
                                        "Please use the more recent version of digiKam that you used before.");

                d->dbAccess->setLastError(errorMsg);

                if (d->observer)
                {
                    d->observer->error(errorMsg);
                    d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
                }

                return false;
            }
        }
        else
        {
            return makeUpdates();
        }
    }
    else
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "Face database: no database file available";

        DbEngineParameters parameters = d->dbAccess->parameters();

        // No legacy handling: start with a fresh db
        if (!createDatabase())
        {
            QString errorMsg = i18n("Failed to create tables in database.\n%1",
                                    d->dbAccess->backend()->lastError());
            d->dbAccess->setLastError(errorMsg);

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        return true;
    }
}

bool FaceDbSchemaUpdater::makeUpdates()
{
    if (d->currentVersion < schemaVersion())
    {
        if (d->currentVersion == 1)
        {
            updateV1ToV2();
        }
        else if (d->currentVersion == 2)
        {
            updateV2ToV3();
        }
    }

    return true;
}


bool FaceDbSchemaUpdater::createDatabase()
{
    if ( createTables() && createIndices() && createTriggers())
    {
        d->currentVersion         = schemaVersion();
        d->currentRequiredVersion = 3;
        return true;
    }
    else
    {
        return false;
    }
}

bool FaceDbSchemaUpdater::createTables()
{
    return d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceDB"))) &&
           d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceDBOpenCVLBPH"))) &&
           d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceDBFaceMatrices")));
}

bool FaceDbSchemaUpdater::createIndices()
{
    return d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceIndices")));
}

bool FaceDbSchemaUpdater::createTriggers()
{
    return d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceTriggers")));
}

bool FaceDbSchemaUpdater::updateV1ToV2()
{
/*
    if (!d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction("UpdateDBSchemaFromV1ToV2")))
    {
        qError() << "Schema upgrade in DB from V1 to V2 failed!";
        return false;
    }
*/

    d->currentVersion         = 2;
    d->currentRequiredVersion = 1;
    return true;
}

bool FaceDbSchemaUpdater::updateV2ToV3()
{
    d->currentVersion         = 3;
    d->currentRequiredVersion = 3;
    d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceDBFaceMatrices")));

    return true;
}

} // namespace Digikam
