/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Face database schema updater
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class FaceDbSchemaUpdater::Private
{
public:

    Private()
        : setError(false),
          currentVersion(0),
          currentRequiredVersion(0),
          access(0),
          observer(0)
    {
    }

    bool                    setError;

    int                     currentVersion;
    int                     currentRequiredVersion;

    FaceDbAccess*           access;

    InitializationObserver* observer;
};

FaceDbSchemaUpdater::FaceDbSchemaUpdater(FaceDbAccess* const access)
    : d(new Private)
{
    d->access = access;
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
        d->access->db()->setSetting(QString::fromLatin1("DBFaceVersion"), QString::number(d->currentVersion));
    }

    if (d->currentRequiredVersion)
    {
        d->access->db()->setSetting(QString::fromLatin1("DBFaceVersionRequired"), QString::number(d->currentRequiredVersion));
    }

    return success;
}

bool FaceDbSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = d->access->backend()->tables();

    if (tables.contains(QString::fromLatin1("Identities"), Qt::CaseInsensitive))
    {
        // Find out schema version of db file
        QString version         = d->access->db()->setting(QString::fromLatin1("DBFaceVersion"));
        QString versionRequired = d->access->db()->setting(QString::fromLatin1("DBFaceVersionRequired"));
        qCDebug(DIGIKAM_FACEDB_LOG) << "Face database: have a structure version " << version;

        // mini schema update
        if (version.isEmpty() && d->access->parameters().isSQLite())
        {
            version = d->access->db()->setting(QString::fromLatin1("DBVersion"));
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

            d->access->setLastError(errorMsg);

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

                d->access->setLastError(errorMsg);

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

        DbEngineParameters parameters = d->access->parameters();

        // No legacy handling: start with a fresh db
        if (!createDatabase())
        {
            QString errorMsg = i18n("Failed to create tables in database.\n%1",
                                    d->access->backend()->lastError());
            d->access->setLastError(errorMsg);

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
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateFaceDB"))) &&
           d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateFaceDBOpenCVLBPH"))) &&
           d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateFaceDBFaceMatrices")));
}

bool FaceDbSchemaUpdater::createIndices()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateFaceIndices")));
}

bool FaceDbSchemaUpdater::createTriggers()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateFaceTriggers")));
}

bool FaceDbSchemaUpdater::updateV1ToV2()
{
/*
    if (!d->access->backend()->execDBAction(d->access->backend()->getDBAction("UpdateDBSchemaFromV1ToV2")))
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
    d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateFaceDBFaceMatrices")));
    return true;
}

} // namespace Digikam
